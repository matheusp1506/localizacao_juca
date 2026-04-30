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
    esp_err_t ret;

    mpu6050_quaternion_t quaternion;

    wheel_Init();

    while(1) {
        wheel_GetEndoderPulses((int *)&data_to_send.pulses_left, (int *)&data_to_send.pulses_right);
		
		ret = mpu6050_dmp_read_quaternion(I2C_NUM_0, &quaternion);
        if (ret == ESP_OK) {
            data_to_send.quat_x = quaternion.x;
            data_to_send.quat_y = quaternion.y;
            data_to_send.quat_z = quaternion.z;
            data_to_send.quat_w = quaternion.w;
            // Envia para a fila. Se a fila estiver cheia, espera 10ms
            xQueueSend(xRobotDataQueue, &data_to_send, pdMS_TO_TICKS(10));
        } else {
            //ESP_LOGE("SENSOR_TASK", "Failed to read quaternion from MPU6050");
            
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Frequência de amostragem
    }
}
