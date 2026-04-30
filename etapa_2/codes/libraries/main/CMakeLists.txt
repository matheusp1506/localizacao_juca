idf_component_register(SRCS "threeeyes.c" "wheel.c" "main.c"  "mpu6050.c" "test_tasks/ir_line_task" "test_tasks/threeeyes_task.c" "test_tasks/imu_task.c" "test_tasks/wheel_task.c" "webserver.c" "wifi_manager.c"
                    INCLUDE_DIRS "." "./test_tasks"
                    EMBED_FILES "index.html"
                    REQUIRES bdc_motor driver esp_adc esp_timer esp_http_server esp_wifi nvs_flash esp_event)
