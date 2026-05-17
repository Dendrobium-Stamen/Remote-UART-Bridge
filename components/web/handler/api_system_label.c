#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "espnow_manager.h"

#include "web_tools.h"

#define WEB_SYSTEM_LABEL_PEER_PARSE_PEER_MAC_BUFFER_LENGTH 1024 * 2

static const char *TAG = "Web system label handler";

esp_err_t system_label_handler(httpd_req_t *req)
{
    char buf[WEB_SYSTEM_LABEL_PEER_PARSE_PEER_MAC_BUFFER_LENGTH];
    int len = web_tools_read_body(req, buf, sizeof(buf));
    if (len <= 0)
        return web_tools_send_json_error(req, "empty body");

    cJSON *root = cJSON_Parse(buf);
    if (!root)
        return web_tools_send_json_error(req, "invalid json");

    cJSON *jlabel = cJSON_GetObjectItem(root, "label");
    if (!cJSON_IsString(jlabel) || !jlabel->valuestring[0])
    {
        cJSON_Delete(root);
        return web_tools_send_json_error(req, "label required");
    }

    uint8_t label_len = (uint8_t)strlen(jlabel->valuestring);
    if (label_len > ESPNOW_MANAGER_MAX_LABEL_LENGTH)
    {
        cJSON_Delete(root);
        return web_tools_send_json_error(req, "label too long");
    }

    ESP_LOGI(TAG, "label %s, label length :%d", jlabel->valuestring, label_len);

    espnow_manager_error_t err = espnow_manager_set_label(jlabel->valuestring, label_len);
    cJSON_Delete(root);

    if (err != ESPNOW_MANAGER_OK)
        return web_tools_send_json_error(req, "label set failed");

    return web_tools_send_json_ok(req);
}
