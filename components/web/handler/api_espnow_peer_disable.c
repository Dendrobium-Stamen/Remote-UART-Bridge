#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "espnow_manager.h"

#include "web_tools.h"

static const char *TAG = "api_peer_disable";

#define WEB_MAC_DISABLE_PEER_PARSE_PEER_MAC_BUFFER_LENGTH 1024 * 2

static bool parse_peer_mac_req(httpd_req_t *req, peer_mac_req_t *out)
{
    char buf[WEB_MAC_DISABLE_PEER_PARSE_PEER_MAC_BUFFER_LENGTH];
    if (web_tools_read_body(req, buf, sizeof(buf)) <= 0)
        return false;

    cJSON *root = cJSON_Parse(buf);
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

esp_err_t espnow_peer_disable_handler(httpd_req_t *req)
{
    peer_mac_req_t body = {0};
    if (!parse_peer_mac_req(req, &body))
        return web_tools_send_json_error(req, "Invalid request body");

    uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
    if (!web_tools_str_to_mac(body.mac, mac))
        return web_tools_send_json_error(req, "Invalid MAC address format");

    espnow_manager_error_t err = espnow_manager_disable_mac(mac);
    if (err != ESPNOW_MANAGER_OK)
    {
        ESP_LOGE(TAG, "disable_mac failed: %d", err);
        return web_tools_send_json_error(req, "Failed to disable peer");
    }

    ESP_LOGD(TAG, "Peer disabled: %s", body.mac);
    return web_tools_send_json_ok(req);
}
