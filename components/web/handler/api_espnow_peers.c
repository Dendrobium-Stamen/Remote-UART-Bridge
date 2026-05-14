#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "espnow_manager.h"

#include "web_tools.h"

static const char *TAG = "api_peers";

esp_err_t espnow_peers_handler(httpd_req_t *req)
{
    uint8_t count = 0;
    size_t max_label = 0;
    espnow_manager_error_t err = espnow_manager_get_peer_mac_count(&count, &max_label);
    if (err != ESPNOW_MANAGER_OK)
    {
        ESP_LOGE(TAG, "get_peer_mac_count failed: %d", err);
        return web_tools_send_json_error(req, "Failed to get peer count");
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();

    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
        char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH] = {0};

        bool is_enabled = false;
        err = espnow_manager_get_peer_mac(i, mac, label, &is_enabled);
        if (err != ESPNOW_MANAGER_OK)
            break;

        char mac_str[WEB_TOOLS_MAC_TO_STR_LENGTH];
        web_tools_mac_to_str(mac, mac_str);

        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "mac", mac_str);
        cJSON_AddStringToObject(item, "label", label);
        cJSON_AddBoolToObject(item, "enabled", is_enabled);
        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "peers", array);
    esp_err_t ret = web_tools_send_json_resp(req, root);
    cJSON_Delete(root);
    return ret;
}
