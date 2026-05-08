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
    ESP_ERROR_CHECK(httpd_start(&web.server, &httpd_config));

    const httpd_uri_t httpd_uris[] = {
        {.uri = "/", .method = HTTP_GET, .handler = page_handler},
        {.uri = "/api/status", .method = HTTP_GET, .handler = api_status_handler},
        {.uri = "/api/peers", .method = HTTP_GET, .handler = api_peers_handler},
        {.uri = "/api/peer/add", .method = HTTP_POST, .handler = api_peer_add_handler},
        {.uri = "/api/peer/del", .method = HTTP_POST, .handler = api_peer_del_handler},
        {.uri = "/api/scan", .method = HTTP_POST, .handler = api_scan_handler},
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
