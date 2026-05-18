#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "espnow_manager.h"

#include "web_tools.h"

static const char *TAG = "api_peer_del";

#define WEB_DELETE_PEER_PARSE_PEER_MAC_BUFFER_LENGTH 1024 * 2

static bool parse_peer_mac_req(httpd_req_t *req, peer_mac_req_t *out)
{
    char buffer[WEB_DELETE_PEER_PARSE_PEER_MAC_BUFFER_LENGTH];
    if (web_tools_read_body(req, buffer, sizeof(buffer)) <= 0)
        return false;

    cJSON *root = cJSON_Parse(buffer);
    if (!root)
        return false;

    const cJSON *j_mac = cJSON_GetObjectItem(root, "mac");
    if (!cJSON_IsString(j_mac))
    {
        cJSON_Delete(root);
        return false;
    }

    strncpy(out->mac, j_mac->valuestring, WEB_TOOLS_MAC_TO_STR_LENGTH - 1);
    cJSON_Delete(root);
    return true;
}

esp_err_t espnow_peer_delete_handler(httpd_req_t *req)
{
    peer_mac_req_t body = {0};
    if (!parse_peer_mac_req(req, &body))
        return web_tools_send_json_error(req, "Invalid request body");

    uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
    if (!web_tools_str_to_mac(body.mac, mac))
        return web_tools_send_json_error(req, "Invalid MAC address format");

    espnow_manager_error_t err = espnow_manager_del_peer_mac(mac);
    if (err != ESPNOW_MANAGER_OK)
    {
        ESP_LOGE(TAG, "del_peer_mac failed: %d", err);
        return web_tools_send_json_error(req, "Failed to delete peer");
    }

    ESP_LOGI(TAG, "Peer removed: %s", body.mac);
    return web_tools_send_json_ok(req);
}
