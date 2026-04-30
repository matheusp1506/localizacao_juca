#include "mpu6050init.h"
#include <stdio.h>

#define I2C_MASTER_SDA_GPIO GPIO_NUM_40
#define I2C_MASTER_SCL_GPIO GPIO_NUM_39
//#define I2C_MASTER_SDA_GPIO GPIO_NUM_21
//#define I2C_MASTER_SCL_GPIO GPIO_NUM_22
#define I2C_NUM I2C_NUM_0

const static char *TAG = "MPU_INIT";

esp_err_t mpu6050_complete_init() {
    esp_err_t ret;

    // Inicializa o MPU6050
    ret = mpu6050_init(I2C_NUM, I2C_MASTER_SDA_GPIO, I2C_MASTER_SCL_GPIO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize MPU6050");
        vTaskDelete(NULL);
        return ret;
    }

    ret = mpu6050_dmp_initialize(I2C_NUM);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize DMP");
        vTaskDelete(NULL);
        return ret;
    }

    // CALIBRATION START
    ESP_LOGI(TAG, "Calibrating MPU6050... Keep Juca still!");
    // These functions usually take ~400-1000 samples and write to MPU registers
    mpu6050_calibrate_gyro(I2C_NUM, 15); // '15' is a common loop count for calibration
    mpu6050_calibrate_accel(I2C_NUM, 15);
    ESP_LOGI(TAG, "Calibration complete!");

    ret = mpu6050_dmp_enable(I2C_NUM, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable DMP");
        vTaskDelete(NULL);
        return ret;
    }

    return ESP_OK;
}