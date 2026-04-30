#ifndef MPU6050INIT_H
#define MPU6050INIT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mpu6050.h"
#include "esp_log.h"
#include <math.h>

esp_err_t mpu6050_complete_init();

#endif // MPU6050INIT_H