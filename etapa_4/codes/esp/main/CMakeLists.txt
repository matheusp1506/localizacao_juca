idf_component_register(SRCS "wifi_manager.c" "webserver.c" "threeeyes.c" "wheel.c" "main.c"  "mpu6050.c" "test_tasks/ir_line_task" "test_tasks/threeeyes_task.c" "test_tasks/imu_task.c" "test_tasks/wheel_task.c" "test_tasks/sensor_task.c" "test_tasks/estimation_task.c" "mpu6050init.c" "test_tasks/data_task.c" "mqtt_manager.c"
                    INCLUDE_DIRS "." "./test_tasks"
                    REQUIRES bdc_motor driver esp_adc esp_timer esp_http_server esp_wifi nvs_flash esp_event mqtt
                    EMBED_FILES index.html)
