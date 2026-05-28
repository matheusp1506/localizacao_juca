/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include "pid_ctrl.h"
#include "hal/gpio_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "wheel.h"
#include "threeeyes_task.h"
#include "wheel_task.h"
#include "imu_task.h"
#include "estimation_task.h"
#include "sensor_task.h"
#include "data_task.h"
#include "mpu6050init.h"
#include "robotstructures.h"
#include "wifi_manager.h"
//#include "webserver.h"
#include <inttypes.h>

#define TEST_TASKS

//#define THREE_EYES_TASK
//#define IMU_TASK
#define WHEEL_CTRL_TASK
#define ESTIMATION_TASK
#define SENSOR_TASK
#define DATA_TASK

QueueHandle_t xRobotDataQueue = NULL;
QueueHandle_t xOdomRosQueue = NULL;
EventGroupHandle_t s_wifi_event_group = NULL;

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();

    wifi_init_sta();

    ESP_LOGI("main", "Iniciando sistema do robô...");
    xRobotDataQueue = xQueueCreate(5, sizeof(robot_data_t));
    
    if (xRobotDataQueue == NULL) {
        ESP_LOGE("main", "Failed to create xRobotDataQueue");
        return;
    }

    xOdomRosQueue = xQueueCreate(5, sizeof(odom_final_data_t));
    if(xOdomRosQueue == NULL) {
        ESP_LOGE("main", "Failed to create xOdomRosQueue");
        return;
    }

    wheel_Init();

    mpu6050_complete_init();

    //ESP_LOGI("main", "ESP_WIFI_MODE_STA");
    //wifi_init_sta();

    //httpd_handle_t server = start_webserver();

#ifdef THREE_EYES_TASK
    xTaskCreate(Threeeyes,
                "threeeyes",
                configMINIMAL_STACK_SIZE*3,
                NULL,
                5,
                NULL);
#endif

#ifdef IMU_TASK
    xTaskCreate(IMU_Task,
                "imu",
                configMINIMAL_STACK_SIZE*3,
                (void *)server,
                5,
                NULL);
#endif

#ifdef WHEEL_CTRL_TASK
    xTaskCreate(wheel_ctrl,
                "wheel",
                configMINIMAL_STACK_SIZE*3,
                NULL,
                5,
                NULL);
#endif

#ifdef ESTIMATION_TASK
    xTaskCreate(PosEstimation,
                "estimation",
                configMINIMAL_STACK_SIZE*4,
                NULL,
                5,
                NULL);
#endif

#ifdef SENSOR_TASK
    xTaskCreate(SensorReader,
                "sensor_reader",
                configMINIMAL_STACK_SIZE*4,
                NULL,
                5,
                NULL);
#endif

#ifdef DATA_TASK
    xTaskCreate(TaskPublisher,
                "data_publisher",
                configMINIMAL_STACK_SIZE*4,
                NULL,
                5,
                NULL);
#endif
}