#include "webserver.h"

static const char *TAG = "WEB_SERVER";

/* Symbols for the embedded HTML file */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

int client_fd = -1; // -1 means no browser is connected

/* 1. Handler to serve the HTML Page */
esp_err_t get_index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
}

/* 2. Basic WebSocket Handler (Place-holder for now) */
esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        client_fd = httpd_req_to_sockfd(req); // Save the connection ID
        ESP_LOGI(TAG, "Browser connected! ID: %d", client_fd);
        return ESP_OK;
    }
    return ESP_OK;
}

/* 3. Task Configuration and Start */
httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true; // Close oldest connection if memory is low

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Route for the 3D dashboard
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = get_index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        // Route for WebSockets
        httpd_uri_t ws_uri = {
            .uri        = "/ws",
            .method     = HTTP_GET,
            .handler    = ws_handler,
            .user_ctx   = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stream_quaternion_to_web(httpd_handle_t server, float w, float x, float y, float z) {
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 20000) { // If heap is dangerously low
        return; // Drop the frame to keep the ESP32 alive
    }
    
    if (client_fd != -1) {
        char json_buf[128];
        int len = snprintf(json_buf, sizeof(json_buf), 
                           "{\"w\":%.4f,\"x\":%.4f,\"y\":%.4f,\"z\":%.4f}", 
                           w, x, y, z);

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t *)json_buf;
        ws_pkt.len = len;
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        // Push to the browser
        esp_err_t ret = httpd_ws_send_frame_async(server, client_fd, &ws_pkt);
        
        if (ret != ESP_OK) {
            ESP_LOGE("WS", "Failed to send, client probably closed. Resetting FD.");
            client_fd = -1; // Reset so we don't keep trying to send to a dead link
        }
    }
}