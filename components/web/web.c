#include "stdio.h"

#include "esp_log.h"
#include "esp_mac.h"

#include "web.h"
#include "web_mode.h"
#include "web_handler.h"

static const char *TAG = "web_espnow";

web_t web = {};

web_error_t web_init(web_config_t *config)
{
    if (config == NULL)
        return WEB_ERROR_INIT;

    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
    httpd_config.max_uri_handlers = 20;
    httpd_config.stack_size = 1024 * 10;
    ESP_ERROR_CHECK(httpd_start(&web.server, &httpd_config));

    const httpd_uri_t httpd_uris[] = {
        {.uri = "/", .method = HTTP_GET, .handler = espnow_page_handler},
        {.uri = "/espnow", .method = HTTP_GET, .handler = espnow_page_handler},
        {.uri = "/monitor", .method = HTTP_GET, .handler = monitor_page_handler},

        {.uri = "/api/system/status", .method = HTTP_GET, .handler = system_status_handler},
        {.uri = "/api/system/label", .method = HTTP_POST, .handler = system_label_handler},
        {.uri = "/api/ota/upload", .method = HTTP_POST, .handler = ota_upload_handler},

        {.uri = "/api/espnow/peers", .method = HTTP_GET, .handler = espnow_peers_handler},
        {.uri = "/api/espnow/scan", .method = HTTP_POST, .handler = espnow_scan_handler},
        {.uri = "/api/espnow/peer/add", .method = HTTP_POST, .handler = espnow_peer_add_handler},
        {.uri = "/api/espnow/peer/delete", .method = HTTP_POST, .handler = espnow_peer_delete_handler},
        {.uri = "/api/espnow/peer/enable", .method = HTTP_POST, .handler = espnow_peer_enable_handler},
        {.uri = "/api/espnow/peer/disable", .method = HTTP_POST, .handler = espnow_peer_disable_handler},
        // {.uri = "/api/espnow/test/send", .method = HTTP_POST, .handler = espnow_test_send_handler},
    };

    for (int i = 0; i < sizeof(httpd_uris) / sizeof(httpd_uris[0]); i++)
    {
        httpd_register_uri_handler(web.server, &httpd_uris[i]);
    }

    ESP_LOGI(TAG, "ESP-NOW Web Config started on port %d", httpd_config.server_port);
    return WEB_OK;
}

web_error_t web_deinit(void)
{
    // httpd_stop();
    return WEB_OK;
}
