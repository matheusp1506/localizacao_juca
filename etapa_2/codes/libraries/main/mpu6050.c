#include "mpu6050.h"
#include "mpu6050_dmp_firmware.h"
#include <math.h>

static const char *TAG = "MPU6050";

#define MPU6050_DMP_MEMORY_CHUNK_SIZE 16

esp_err_t mpu6050_write_register(i2c_port_t i2c_num, uint8_t reg, uint8_t data);
esp_err_t mpu6050_read_registers(i2c_port_t i2c_num, uint8_t reg, uint8_t *data, size_t len);


// Funções até o traço são adaptadas da biblioteca: I2Cdev library collection - MPU6050 I2C device class
// A biblioteca original é de autoria de: 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// Função que escreve múltiplos registradores
static esp_err_t mpu6050_write_registers(i2c_port_t i2c_num, uint8_t reg, const uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_write(cmd, (uint8_t *)data, len, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Função que permite a atualização de um número limitado de bits do registrador
static esp_err_t mpu6050_update_register_bits(i2c_port_t i2c_num, uint8_t reg, uint8_t mask, uint8_t value) {
    uint8_t current = 0;
    esp_err_t ret = mpu6050_read_registers(i2c_num, reg, &current, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    current = (current & ~mask) | (value & mask);
    return mpu6050_write_register(i2c_num, reg, current);
}

static esp_err_t mpu6050_set_memory_bank(i2c_port_t i2c_num, uint8_t bank, bool prefetch_enabled, bool user_bank) {
    bank &= 0x1F;
    if (user_bank) {
        bank |= 0x20;
    }
    if (prefetch_enabled) {
        bank |= 0x40;
    }
    return mpu6050_write_register(i2c_num, MPU6050_REG_BANK_SEL, bank);
}

static esp_err_t mpu6050_set_memory_start_address(i2c_port_t i2c_num, uint8_t address) {
    return mpu6050_write_register(i2c_num, MPU6050_REG_MEM_START, address);
}

static esp_err_t mpu6050_read_word(i2c_port_t i2c_num, uint8_t reg, int16_t *value) {
    uint8_t raw[2] = {0};
    esp_err_t ret = mpu6050_read_registers(i2c_num, reg, raw, 2);
    if (ret != ESP_OK) {
        return ret;
    }
    *value = (int16_t)((raw[0] << 8) | raw[1]);
    return ESP_OK;
}

static esp_err_t mpu6050_write_word(i2c_port_t i2c_num, uint8_t reg, int16_t value) {
    uint8_t raw[2] = {(uint8_t)((value >> 8) & 0xFF), (uint8_t)(value & 0xFF)};
    return mpu6050_write_registers(i2c_num, reg, raw, 2);
}

static float mpu6050_mapf(float val, float in_min, float in_max, float out_min, float out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static esp_err_t mpu6050_calibrate_pid(i2c_port_t i2c_num, uint8_t read_address, float kP, float kI, uint8_t loops) {
    const uint8_t save_address = (read_address == MPU6050_REG_ACCEL_XOUT_H) ? MPU6050_REG_XA_OFFS_H : MPU6050_REG_XG_OFFS_USRH;
    int16_t data = 0;
    int16_t bit_zero[3] = {0};
    float i_term[3] = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 3; i++) {
        esp_err_t ret = mpu6050_read_word(i2c_num, (uint8_t)(save_address + (i * 2)), &data);
        if (ret != ESP_OK) {
            return ret;
        }
        if (save_address != MPU6050_REG_XG_OFFS_USRH) {
            bit_zero[i] = data & 1;
            i_term[i] = (float)data * 8.0f;
        } else {
            i_term[i] = (float)data * 4.0f;
        }
    }

    for (uint8_t l = 0; l < loops; l++) {
        int16_t e_sample = 0;

        for (int c = 0; c < 100; c++) {
            float e_sum = 0.0f;

            for (int i = 0; i < 3; i++) {
                esp_err_t ret = mpu6050_read_word(i2c_num, (uint8_t)(read_address + (i * 2)), &data);
                if (ret != ESP_OK) {
                    return ret;
                }

                float reading = (float)data;
                if ((read_address == MPU6050_REG_ACCEL_XOUT_H) && (i == 2)) {
                    reading -= 16384.0f;
                }

                const float error = -reading;
                e_sum += fabsf(reading);

                const float p_term = kP * error;
                i_term[i] += (error * 0.001f) * kI;

                if (save_address != MPU6050_REG_XG_OFFS_USRH) {
                    data = (int16_t)lroundf((p_term + i_term[i]) / 8.0f);
                    data = (int16_t)((data & 0xFFFE) | bit_zero[i]);
                } else {
                    data = (int16_t)lroundf((p_term + i_term[i]) / 4.0f);
                }

                ret = mpu6050_write_word(i2c_num, (uint8_t)(save_address + (i * 2)), data);
                if (ret != ESP_OK) {
                    return ret;
                }
            }

            if ((c == 99) && (e_sum > 1000.0f)) {
                c = 0;
            }

            if ((e_sum * ((read_address == MPU6050_REG_ACCEL_XOUT_H) ? 0.05f : 1.0f)) < 5.0f) {
                e_sample++;
            }
            if ((e_sum < 100.0f) && (c > 10) && (e_sample >= 10)) {
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(1));
        }

        kP *= 0.75f;
        kI *= 0.75f;

        for (int i = 0; i < 3; i++) {
            if (save_address != MPU6050_REG_XG_OFFS_USRH) {
                data = (int16_t)lroundf(i_term[i] / 8.0f);
                data = (int16_t)((data & 0xFFFE) | bit_zero[i]);
            } else {
                data = (int16_t)lroundf(i_term[i] / 4.0f);
            }

            esp_err_t ret = mpu6050_write_word(i2c_num, (uint8_t)(save_address + (i * 2)), data);
            if (ret != ESP_OK) {
                return ret;
            }
        }
    }

    esp_err_t ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 2), (1 << 2));
    if (ret != ESP_OK) {
        return ret;
    }

    return mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 3), (1 << 3));
}

static esp_err_t mpu6050_write_memory_block(i2c_port_t i2c_num,
                                            const uint8_t *data,
                                            uint16_t data_size,
                                            uint8_t bank,
                                            uint8_t address) {
    uint16_t i = 0;
    while (i < data_size) {
        uint16_t chunk_size = MPU6050_DMP_MEMORY_CHUNK_SIZE;
        if (i + chunk_size > data_size) {
            chunk_size = data_size - i;
        }
        if (chunk_size > (uint16_t)(256 - address)) {
            chunk_size = (uint16_t)(256 - address);
        }

        if (chunk_size == 0) {
            ESP_LOGE(TAG, "Invalid zero-length DMP memory chunk at bank %u address %u", bank, address);
            return ESP_ERR_INVALID_SIZE;
        }

        esp_err_t ret = mpu6050_set_memory_bank(i2c_num, bank, false, false);
        if (ret != ESP_OK) {
            return ret;
        }

        ret = mpu6050_set_memory_start_address(i2c_num, address);
        if (ret != ESP_OK) {
            return ret;
        }

        ret = mpu6050_write_registers(i2c_num, MPU6050_REG_MEM_R_W, data + i, chunk_size);
        if (ret != ESP_OK) {
            return ret;
        }

        i += chunk_size;
        address += chunk_size;
        if (address == 0) {
            bank++;
        }
    }

    return ESP_OK;
}

static esp_err_t mpu6050_write_dmp_config_set(i2c_port_t i2c_num, const uint8_t *data, uint16_t data_size) {
    uint16_t i = 0;
    while (i < data_size) {
        uint8_t bank = data[i++];
        uint8_t offset = data[i++];
        uint8_t length = data[i++];

        if (length > 0) {
            esp_err_t ret = mpu6050_write_memory_block(i2c_num, data + i, length, bank, offset);
            if (ret != ESP_OK) {
                return ret;
            }
            i += length;
        } else {
            uint8_t special = data[i++];
            if (special == 0x01) {
                esp_err_t ret = mpu6050_write_register(i2c_num, MPU6050_REG_INT_ENABLE, 0x32);
                if (ret != ESP_OK) {
                    return ret;
                }
            }
        }
    }

    return ESP_OK;
}
// ----------------------------------------------------

// Função para escrever um byte no MPU6050
esp_err_t mpu6050_write_register(i2c_port_t i2c_num, uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_write_byte(cmd, data, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Função para ler múltiplos bytes do MPU6050
esp_err_t mpu6050_read_registers(i2c_port_t i2c_num, uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_start(cmd);  // Repeated start
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, 1);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Função de inicialização do MPU6050
esp_err_t mpu6050_init(i2c_port_t i2c_num, gpio_num_t sda_gpio, gpio_num_t scl_gpio) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000  // 100kHz
    };

    esp_err_t ret = i2c_param_config(i2c_num, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed");
        return ret;
    }

    ret = i2c_driver_install(i2c_num, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
        return ret;
    }

    // Acorda o MPU6050 e inicializa o sensor
    ret = mpu6050_write_register(i2c_num, MPU6050_REG_PWR_MGMT_1, 0x00);  // Acorda o sensor
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6050");
        return ret;
    }

    return ESP_OK;
}

// Função para ler dados de aceleração e giroscópio
esp_err_t mpu6050_read_data(i2c_port_t i2c_num, mpu6050_data_t *data) {
    uint8_t buffer[14];
    esp_err_t ret = mpu6050_read_registers(i2c_num, MPU6050_REG_ACCEL_XOUT_H, buffer, 14);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read MPU6050 data");
        return ret;
    }

    // Aceleração
    data->accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);

    // Giroscópio
    data->gyro_x = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gyro_y = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gyro_z = (int16_t)((buffer[12] << 8) | buffer[13]);

    return ESP_OK;
}

// Funções a partir deste ponto são construídas seguindo a biblioteca: I2Cdev library collection - MPU6050 I2C device class
// A biblioteca original é de autoria de: 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// Função para resetar o MPU-6050
esp_err_t mpu6050_reset(i2c_port_t i2c_num) {
    uint8_t reg_prev_data = 0;
    esp_err_t ret = mpu6050_read_registers(i2c_num, MPU6050_REG_PWR_MGMT_1, &reg_prev_data, 1);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read power register");
        return ret;
    }
    reg_prev_data |= (1<<7);
    ret = mpu6050_write_register(i2c_num, MPU6050_REG_PWR_MGMT_1, reg_prev_data);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset MPU6050");
        return ret;
    }
    return ESP_OK;
}

// Seta o status do modo de dormir.
esp_err_t mpu6050_set_sleep_enabled(i2c_port_t i2c_num, uint16_t enabled) {
    uint8_t reg_prev_data = 0;
    esp_err_t ret = mpu6050_read_registers(i2c_num, MPU6050_REG_PWR_MGMT_1, &reg_prev_data, 1);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read power register");
        return ret;
    }
    
    if(enabled) {
        reg_prev_data |= (1<<6);
    } else {
        reg_prev_data &= ~(1<<6);
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_PWR_MGMT_1, reg_prev_data);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set sleep");
        return ret;
    }
    return ESP_OK;
}

// Inicializa a DMP e prepara FIFO para pacote de 42 bytes com quaternion.
esp_err_t mpu6050_dmp_initialize(i2c_port_t i2c_num) {
    esp_err_t ret = mpu6050_reset(i2c_num);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset before DMP init");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(30));

    ret = mpu6050_set_sleep_enabled(i2c_num, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable sleep before DMP init");
        return ret;
    }

    ret = mpu6050_set_memory_bank(i2c_num, 0x10, true, true);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = mpu6050_set_memory_start_address(i2c_num, 0x06);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_set_memory_bank(i2c_num, 0x00, false, false);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_memory_block(i2c_num, dmpMemory, MPU6050_DMP_CODE_SIZE, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write DMP memory block");
        return ret;
    }

    ret = mpu6050_write_dmp_config_set(i2c_num, dmpConfig, MPU6050_DMP_CONFIG_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write DMP config");
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_PWR_MGMT_1, 0x07, 0x03);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_INT_ENABLE, 0x12);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_SMPLRT_DIV, 0x04);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_CONFIG, 0x38, 0x08);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_CONFIG, 0x07, 0x03);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_GYRO_CONFIG, 0x18, 0x18);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_DMP_CFG_1, 0x03);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_DMP_CFG_2, 0x00);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_MOT_THR, 2);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_ZRMOT_THR, 156);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_MOT_DUR, 80);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_write_register(i2c_num, MPU6050_REG_ZRMOT_DUR, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 2), (1 << 2));
    if (ret != ESP_OK) {
        return ret;
    }
    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 3), (1 << 3));
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 6), (1 << 6));
    if (ret != ESP_OK) {
        return ret;
    }
    ret = mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 7), 0);
    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t int_status = 0;
    ret = mpu6050_read_registers(i2c_num, MPU6050_REG_INT_STATUS, &int_status, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

esp_err_t mpu6050_dmp_enable(i2c_port_t i2c_num, bool enabled) {
    uint8_t value = enabled ? (1 << 7) : 0;
    return mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 7), value);
}

static esp_err_t mpu6050_dmp_read_fifo_packet(i2c_port_t i2c_num, uint8_t *fifo_packet) {
    uint8_t int_status = 0;
    uint8_t fifo_count_buffer[2] = {0};

    esp_err_t ret = mpu6050_read_registers(i2c_num, MPU6050_REG_INT_STATUS, &int_status, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = mpu6050_read_registers(i2c_num, MPU6050_REG_FIFO_COUNTH, fifo_count_buffer, 2);
    if (ret != ESP_OK) {
        return ret;
    }

    uint16_t fifo_count = ((uint16_t)fifo_count_buffer[0] << 8) | fifo_count_buffer[1];
    if ((int_status & 0x10) || fifo_count == 1024) {
        mpu6050_update_register_bits(i2c_num, MPU6050_REG_USER_CTRL, (1 << 2), (1 << 2));
        return ESP_ERR_INVALID_STATE;
    }

    if ((int_status & 0x02) == 0) {
        return ESP_ERR_NOT_FOUND;
    }

    while (fifo_count < MPU6050_DMP_PACKET_SIZE) {
        ret = mpu6050_read_registers(i2c_num, MPU6050_REG_FIFO_COUNTH, fifo_count_buffer, 2);
        if (ret != ESP_OK) {
            return ret;
        }
        fifo_count = ((uint16_t)fifo_count_buffer[0] << 8) | fifo_count_buffer[1];
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    ret = mpu6050_read_registers(i2c_num, MPU6050_REG_FIFO_R_W, fifo_packet, MPU6050_DMP_PACKET_SIZE);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

esp_err_t mpu6050_dmp_read_quaternion(i2c_port_t i2c_num, mpu6050_quaternion_t *q) {
    if (q == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t packet[MPU6050_DMP_PACKET_SIZE] = {0};
    esp_err_t ret = mpu6050_dmp_read_fifo_packet(i2c_num, packet);
    if (ret != ESP_OK) {
        return ret;
    }

    int16_t q_i[4];
    q_i[0] = (int16_t)((packet[0] << 8) | packet[1]);
    q_i[1] = (int16_t)((packet[4] << 8) | packet[5]);
    q_i[2] = (int16_t)((packet[8] << 8) | packet[9]);
    q_i[3] = (int16_t)((packet[12] << 8) | packet[13]);

    q->w = (float)q_i[0] / 16384.0f;
    q->x = (float)q_i[1] / 16384.0f;
    q->y = (float)q_i[2] / 16384.0f;
    q->z = (float)q_i[3] / 16384.0f;

    return ESP_OK;
}

esp_err_t mpu6050_calibrate_gyro(i2c_port_t i2c_num, uint8_t loops) {
    if (loops == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    float kP = 0.3f;
    float kI = 90.0f;
    const float scale = (100.0f - mpu6050_mapf((float)loops, 1.0f, 5.0f, 20.0f, 0.0f)) * 0.01f;
    kP *= scale;
    kI *= scale;

    ESP_LOGI(TAG, "Calibrating gyro with %u loops", loops);
    return mpu6050_calibrate_pid(i2c_num, MPU6050_REG_GYRO_XOUT_H, kP, kI, loops);
}

esp_err_t mpu6050_calibrate_accel(i2c_port_t i2c_num, uint8_t loops) {
    if (loops == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    float kP = 0.3f;
    float kI = 20.0f;
    const float scale = (100.0f - mpu6050_mapf((float)loops, 1.0f, 5.0f, 20.0f, 0.0f)) * 0.01f;
    kP *= scale;
    kI *= scale;

    ESP_LOGI(TAG, "Calibrating accel with %u loops", loops);
    return mpu6050_calibrate_pid(i2c_num, MPU6050_REG_ACCEL_XOUT_H, kP, kI, loops);
}