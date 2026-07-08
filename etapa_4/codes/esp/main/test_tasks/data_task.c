#include "data_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "robotstructures.h"
#include "mqtt_client.h"
#include "webserver.h"

portTASK_FUNCTION(TaskPublisher, arg) {
    odom_final_data_t data_received;

    while (s_wifi_event_group == NULL) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_DEBUG);
esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_DEBUG);
esp_log_level_set("TRANSPORT_BASE", ESP_LOG_DEBUG);

    while(1) {
        EventBits_t bits = xEventGroupWaitBits(
            s_wifi_event_group,        // The event group handle
            WIFI_CONNECTED_BIT, // Bits to wait for
            pdFALSE,              // Should bits be cleared before returning?
            pdTRUE,               // Wait for ALL bits? (pdFALSE = Wait for ANY)
            portMAX_DELAY         // How long to wait
        );

        if(bits & (WIFI_CONNECTED_BIT)) {
            ESP_LOGI("PUBLISHER_TASK", "Wi-Fi connected, starting to publish data...");
            break; // Exit the loop and start publishing
        } else {
            ESP_LOGW("PUBLISHER_TASK", "Waiting for Wi-Fi connection...");
        }
    }

    httpd_handle_t server = start_webserver();
    esp_err_t ret = mqtt_start_client();

    if(ret != ESP_OK) {
        ESP_LOGE("PUBLISHER_TASK", "Failed to start MQTT client");
        vTaskDelete(NULL);
        return;
    }

    while(1) {
        if (xQueueReceive(xOdomRosQueue, &data_received, portMAX_DELAY)) {
            // para publicar no tópico /odom
            /*ESP_LOGI("PUBLISHER_TASK", "Publicando no tópico /odom: x=%.3f m, y=%.3f m, quat=(%.3f, %.3f, %.3f, %.3f), linear_vel=%.3f m/s, angular_vel=%.3f rad/s",
                     data_received.x, data_received.y,
                     data_received.quat_x, data_received.quat_y, data_received.quat_z, data_received.quat_w,
                     data_received.linear_velocity, data_received.angular_velocity);
            */
            // Publicar no MQTT
            char mqtt_payload[256];
            int len = snprintf(mqtt_payload, sizeof(mqtt_payload),
                               "{\"x\":%.3f,\"y\":%.3f,\"yaw_enc\":%.3f,\"quat_x\":%.3f,\"quat_y\":%.3f,\"quat_z\":%.3f,\"quat_w\":%.3f,\"linear_velocity\":%.3f,\"angular_velocity\":%.3f,\"pulses_left\":%ld,\"pulses_right\":%ld}",
                               data_received.x, data_received.y, data_received.yaw_encoder,
                               data_received.quat_x, data_received.quat_y, data_received.quat_z, data_received.quat_w,
                               data_received.linear_velocity, data_received.angular_velocity, data_received.pulses_left, data_received.pulses_right);
            esp_mqtt_client_publish(s_mqtt_client, MQTT_QUATERNION_TOPIC, mqtt_payload, len, 1, 0);

            stream_quaternion_to_web(server, data_received.quat_w, data_received.quat_x, data_received.quat_y, data_received.quat_z);
        }
    }

}