#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "message_manager.h"

#include "web_tools.h"

static const char *TAG = "http_server";

esp_err_t espnow_scan_handler(httpd_req_t *req)
{
    message_manager_error_t err = message_manager_scan();
    if (err != MESSAGE_MANAGER_OK)
    {
        ESP_LOGE(TAG, "message_manager_scan failed: %d", err);
        return web_tools_send_json_error(req, "Scan failed");
    }

    message_manager_scan_result_t *result = message_manager_get_scan_result(500);
    if (!result)
        return web_tools_send_json_error(req, "Scan timeout");

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();

    for (size_t i = 0; i < result->device_count; i++)
    {
        char mac_str[WEB_TOOLS_MAC_TO_STR_LENGTH];
        web_tools_mac_to_str(result->devices[i].mac, mac_str);

        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "mac", mac_str);
        cJSON_AddStringToObject(item, "label", result->devices[i].label);
        cJSON_AddNumberToObject(item, "rssi", result->devices[i].rssi);
        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "devices", array);
    esp_err_t ret = web_tools_send_json_resp(req, root);
    cJSON_Delete(root);
    return ret;
}
