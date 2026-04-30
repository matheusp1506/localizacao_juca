#include <esp_http_server.h>
#include <esp_log.h>
#include "esp_event.h"

esp_err_t get_index_handler(httpd_req_t *req);

esp_err_t ws_handler(httpd_req_t *req);

httpd_handle_t start_webserver(void);

void stream_quaternion_to_web(httpd_handle_t server, float w, float x, float y, float z);