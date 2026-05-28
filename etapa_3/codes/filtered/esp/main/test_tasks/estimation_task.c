#include <stdbool.h>
//#include "../components/espressif__bdc_motor/include/bdc_motor.h"
//#include "../components/espressif__pid_ctrl/include/pid_ctrl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
//#include "esp_private/esp_clk.h"
//#include "driver/mcpwm_cap.h"
//#include "driver/gpio.h"
//#include "portmacro.h"
#include "esp_timer.h"
//#include "driver/pulse_cnt.h"
//#include "hal/gpio_types.h"
//#include "esp_adc/adc_oneshot.h"
//#include "esp_adc/adc_cali.h"
//#include "esp_adc/adc_cali_scheme.h"
#include "wheel.h"
#include <math.h>
#include "robotstructures.h"
#include "estimation_task.h"

const static char *TAG = "ESTIMATION_TASK";
// -----------------ALGUNS DEFINES DO CARRINHO----------------------------------
#define BASE_SPEED (BDC_MCPWM_DUTY_TICK_MAX / 2)
//#define OBSTACLE_THRESHOLD_CM 20.0f
// --- CONSTANTES FÍSICAS DO JUCA ---
//#define WHEEL_RADIUS    0.033f   // Raio da roda em metros (33mm)
//#define TRACK_WIDTH     0.150f   // Distância entre rodas em metros (15cm)
//#define ENCODER_PPR     330.0f   // Pulsos por volta total

// -----------------DEFINIÇÕES DE STRUCT E QUEUE--------------------------------
/*typedef struct {
    float x;
    float y;
    float quat_z;
    float quat_w;
    float linear_velocity;
    float angular_velocity;
} odom_final_data_t;
*/
//QueueHandle_t xOdomRosQueue = NULL; // Fila de saída

/*typedef struct {
    int32_t pulses_left;
    int32_t pulses_right;
	float quat_z;
	float quat_w;
    float gyro_z;          // Velocidade angular do MPU6050
    float dt;              // Tempo entre leituras
} robot_data_t;
*/
//QueueHandle_t xRobotDataQueue = NULL; // Fila para as contas

//-------------------------------------------------------------------------------
/*
void vTaskSensorReader(void *pvParameters) {
    robot_data_t data_to_send;

    while(1) {
        wheel_GetEndoderPulses((int *)&data_to_send.pulses_left, (int *)&data_to_send.pulses_right);
		
		
		
        // Envia para a fila. Se a fila estiver cheia, espera 10ms
        xQueueSend(xRobotDataQueue, &data_to_send, pdMS_TO_TICKS(10));

        vTaskDelay(pdMS_TO_TICKS(50)); // Frequência de amostragem
    }
}
*/

//void vTaskPosEstimation(void *pvParameters) {
portTASK_FUNCTION(PosEstimation, args) {
	robot_data_t local_data; 

    // Estado do Juca (Pose Global)
    float x = 0.0f;
    float y = 0.0f;
	float theta_enc = 0.0f;

    // Guardar leituras anteriores para o Delta D
    int32_t last_pulses_l = 0;
    int32_t last_pulses_r = 0;
	//float last_theta = 0;
	
	float delta_s = 0.0f;
	
	bool first_run = true;
	
    // Constante de conversão
    const float pulse_to_m = (2.0f * M_PI * WHEEL_RADIUS) / ENCODER_PPR;

    while (1) {
        if (xQueueReceive(xRobotDataQueue, &local_data, portMAX_DELAY) == pdPASS) {
			
			if (first_run) {
			    last_pulses_l = local_data.pulses_left;
			    last_pulses_r = local_data.pulses_right;
			    first_run = false;
			    continue;
			}
            
			// Diferença de pulsos (Delta de ticks)
			int32_t diff_l = local_data.pulses_left - last_pulses_l;
			int32_t diff_r = local_data.pulses_right - last_pulses_r;

			// Conversão para metros dos pulsos
			float dist_l = (float)diff_l * pulse_to_m;
			float dist_r = (float)diff_r * pulse_to_m;
			
			// DeltaD e DeltaTheta
			float delta_d = (dist_l + dist_r) / 2.0f;
			float delta_theta = (dist_r - dist_l) / TRACK_WIDTH;
			
			// Normalização do ângulo
            if (delta_theta > M_PI)  delta_theta -= 2.0f * M_PI;
            if (delta_theta < -M_PI) delta_theta += 2.0f * M_PI;
			
			if (fabsf(delta_theta) > 0.0001f) {
			    delta_s = delta_d * (sinf(delta_theta / 2.0f) / (delta_theta / 2.0f));
			} else {
			    delta_s = delta_d;
			}
			
			float delta_x = delta_s * cosf(theta_enc + (delta_theta / 2.0f));
			float delta_y = delta_s * sinf(theta_enc + (delta_theta / 2.0f));
			
			x = x + delta_x;
			y = y + delta_y;

			theta_enc += delta_theta;
			
			// Normalização do theta_enc
            if (theta_enc > M_PI)  theta_enc -= 2.0f * M_PI;
            if (theta_enc < -M_PI) theta_enc += 2.0f * M_PI;
			
			float linear_velocity = delta_d / local_data.dt;
			
			// Atualizaçoes para o proximo ciclo
            last_pulses_l = local_data.pulses_left;
            last_pulses_r = local_data.pulses_right;
			
			// dados para envio
			odom_final_data_t msg_to_send;
	        msg_to_send.x = x;
	        msg_to_send.y = y;
			msg_to_send.yaw_encoder = theta_enc;
			msg_to_send.quat_x = local_data.quat_x;
			msg_to_send.quat_y = local_data.quat_y;
	        msg_to_send.quat_z = local_data.quat_z;
	        msg_to_send.quat_w = local_data.quat_w;
	        msg_to_send.linear_velocity = linear_velocity;
	        msg_to_send.angular_velocity = local_data.gyro_z;

			ESP_LOGI(TAG, "Pose Estimada: x=%.3f m, y=%.3f m, theta=%.3f rad", x, y, theta_enc);
			ESP_LOGI(TAG, "Velocidade Linear: %.3f m/s, Velocidade Angular: %.3f rad/s", linear_velocity, local_data.gyro_z);

	        xQueueSend(xOdomRosQueue, &msg_to_send, 0);
        }
    }
}

/*void vTaskPublisher(void *pvParameters) {
    odom_final_data_t data_received;
    while(1) {
        if (xQueueReceive(xOdomRosQueue, &data_received, portMAX_DELAY)) {
            // para publicar no tópico /odom
        }
    }
}*/

/*void drive_task(void *arg){
    ESP_LOGI("TASK_DRIVE", "Task de controle do robô iniciada.");
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = pdMS_TO_TICKS(20);
	
    wheel_GoForward();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);

}*/

/*
void app_main(void)
{
	ESP_LOGI(TAG, "Iniciando sistema do robô...");

	xRobotDataQueue = xQueueCreate(5, sizeof(robot_data_t));
	xOdomRosQueue   = xQueueCreate(5, sizeof(odom_final_data_t));

	wheel_Init();
	wheel_GoForward();
	wheel_SetVel(0, 0);

	if (xRobotDataQueue != NULL && xOdomRosQueue != NULL) {
	        xTaskCreate(vTaskSensorReader, "Reader", 2048, NULL, 5, NULL);
	        xTaskCreate(vTaskPosEstimation, "Estimator", 4096, NULL, 5, NULL);
	        xTaskCreate(vTaskPublisher, "Publisher", 2048, NULL, 5, NULL);
	}
	
		
		
    xTaskCreate(drive_task, "drive_task", 8192, NULL, 6, NULL);

    ESP_LOGI(TAG, "Sistema inicializado com sucesso. Robô pronto!");
}
*/