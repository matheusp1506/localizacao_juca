#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "robotstructures.h"
#include "sensor_task.h"
#include "mpu6050.h"
#include "wheel.h"

//void vTaskSensorReader(void *pvParameters) {
portTASK_FUNCTION(SensorReader, args) {
    robot_data_t data_to_send;
    uint64_t prev_time, new_time;
    prev_time = esp_timer_get_time();
    esp_err_t ret;

    mpu6050_data_t mpu_data;
    mpu6050_quaternion_t quaternion;

    while(1) {
        wheel_GetEndoderPulses((int *)&data_to_send.pulses_left, (int *)&data_to_send.pulses_right);
		
        data_to_send.pulses_left = -data_to_send.pulses_left; // inverte o sentido do encoder esquerdo, para bater com a convenção de sinais do carrinho
        ESP_LOGI("SENSOR_TASK", "Pulses - Left: %ld, Right: %ld", data_to_send.pulses_left, data_to_send.pulses_right);

		ret = mpu6050_dmp_read_quaternion(I2C_NUM_0, &quaternion);
        if (ret == ESP_OK) {
            data_to_send.quat_x = quaternion.x;
            data_to_send.quat_y = quaternion.y;
            data_to_send.quat_z = quaternion.z;
            data_to_send.quat_w = quaternion.w;
            new_time = esp_timer_get_time();
            data_to_send.dt = (new_time - prev_time) / 1000000.0f; // converte para segundos
            prev_time = new_time;
            mpu6050_read_data(I2C_NUM_0, &mpu_data);
            data_to_send.gyro_z = mpu_data.gyro_z / 131.0f; // converte para rad/s (sensibilidade do giroscópio configurada para ±250°/s)
            // Envia para a fila. Se a fila estiver cheia, espera 10ms
            xQueueSend(xRobotDataQueue, &data_to_send, pdMS_TO_TICKS(10));
        } else {
            //ESP_LOGE("SENSOR_TASK", "Failed to read quaternion from MPU6050");
            
        }

        //vTaskDelay(pdMS_TO_TICKS(50)); // Frequência de amostragem
        vTaskDelay(pdMS_TO_TICKS(20)); // Frequência de amostragem
    }
}