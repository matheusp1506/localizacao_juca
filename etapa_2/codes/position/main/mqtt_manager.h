#include <esp_log.h>
#include "esp_event.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI      "mqtt://10.141.78.214:1883"
#define MQTT_QUATERNION_TOPIC "juca/imu/quaternion"

extern esp_mqtt_client_handle_t s_mqtt_client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t mqtt_start_client(void);