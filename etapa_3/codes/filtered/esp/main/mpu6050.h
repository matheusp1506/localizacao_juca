#ifndef MPU6050_H
#define MPU6050_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include <stdbool.h>

// Endereço I2C do MPU6050
#define MPU6050_ADDR 0x68  // Endereço padrão de 7 bits do MPU6050

// Registradores do MPU6050
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_SMPLRT_DIV   0x19
#define MPU6050_REG_CONFIG       0x1A
#define MPU6050_REG_GYRO_CONFIG  0x1B
#define MPU6050_REG_XA_OFFS_H    0x06
#define MPU6050_REG_YA_OFFS_H    0x08
#define MPU6050_REG_ZA_OFFS_H    0x0A
#define MPU6050_REG_XG_OFFS_USRH 0x13
#define MPU6050_REG_YG_OFFS_USRH 0x15
#define MPU6050_REG_ZG_OFFS_USRH 0x17
#define MPU6050_REG_FIFO_EN      0x23
#define MPU6050_REG_INT_ENABLE   0x38
#define MPU6050_REG_INT_STATUS   0x3A
#define MPU6050_REG_MOT_THR      0x1F
#define MPU6050_REG_MOT_DUR      0x20
#define MPU6050_REG_ZRMOT_THR    0x21
#define MPU6050_REG_ZRMOT_DUR    0x22
#define MPU6050_REG_USER_CTRL    0x6A
#define MPU6050_REG_BANK_SEL     0x6D
#define MPU6050_REG_MEM_START    0x6E
#define MPU6050_REG_MEM_R_W      0x6F
#define MPU6050_REG_DMP_CFG_1    0x70
#define MPU6050_REG_DMP_CFG_2    0x71
#define MPU6050_REG_FIFO_COUNTH  0x72
#define MPU6050_REG_FIFO_R_W     0x74

#define MPU6050_DMP_PACKET_SIZE  42

// Estrutura para armazenar dados de aceleração e giroscópio
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_data_t;

typedef struct {
    float w;
    float x;
    float y;
    float z;
} mpu6050_quaternion_t;

// Função para inicializar o sensor MPU6050
esp_err_t mpu6050_init(i2c_port_t i2c_num, gpio_num_t sda_gpio, gpio_num_t scl_gpio);

// Função para ler dados de aceleração e giroscópio
esp_err_t mpu6050_read_data(i2c_port_t i2c_num, mpu6050_data_t *data);

esp_err_t mpu6050_reset(i2c_port_t i2c_num);
esp_err_t mpu6050_set_sleep_enabled(i2c_port_t i2c_num, uint16_t enabled);
esp_err_t mpu6050_dmp_initialize(i2c_port_t i2c_num);
esp_err_t mpu6050_dmp_enable(i2c_port_t i2c_num, bool enabled);
esp_err_t mpu6050_dmp_read_quaternion(i2c_port_t i2c_num, mpu6050_quaternion_t *q);
esp_err_t mpu6050_calibrate_gyro(i2c_port_t i2c_num, uint8_t loops);
esp_err_t mpu6050_calibrate_accel(i2c_port_t i2c_num, uint8_t loops);

#endif // MPU6050_H
