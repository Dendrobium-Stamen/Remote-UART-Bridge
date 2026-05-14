#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "message_manager.h"

#include "web_tools.h"

static const char *TAG = "Web peer add handler";

typedef struct
{
    char mac[ESPNOW_MANAGER_MAC_LEN];
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];
} peer_add_req_t;

static bool parse_peer_add_req(httpd_req_t *req, peer_add_req_t *out)
{
    char buffer[2048];
    if (web_tools_read_body(req, buffer, sizeof(buffer)) <= 0)
        return false;

    cJSON *root = cJSON_Parse(buffer);
    if (!root)
        return false;

    const cJSON *j_mac = cJSON_GetObjectItem(root, "mac");
    const cJSON *j_label = cJSON_GetObjectItem(root, "label");

    if (!cJSON_IsString(j_mac))
    {
        cJSON_Delete(root);
        return false;
    }

    strncpy(out->mac, j_mac->valuestring, ESPNOW_MANAGER_MAC_LEN - 1);
    if (cJSON_IsString(j_label))
        strncpy(out->label, j_label->valuestring, ESPNOW_MANAGER_MAX_LABEL_LENGTH - 1);
    else
        strncpy(out->label, j_mac->valuestring, ESPNOW_MANAGER_MAX_LABEL_LENGTH - 1);

    cJSON_Delete(root);
    return true;
}

/* ------------------------------------------------------------------ */
/*  POST /api/espnow/peer/add                                          */
/* ------------------------------------------------------------------ */
esp_err_t espnow_peer_add_handler(httpd_req_t *req)
{
    peer_add_req_t body = {0};
    if (!parse_peer_add_req(req, &body))
        return web_tools_send_json_error(req, "Invalid request body");

    uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
    if (!web_tools_str_to_mac(body.mac, mac))
        return web_tools_send_json_error(req, "Invalid MAC address format");

    espnow_manager_error_t err = espnow_manager_add_peer_mac(mac, body.label);
    if (err != ESPNOW_MANAGER_OK)
    {
        ESP_LOGE(TAG, "add_peer_mac failed: %d", err);
        return web_tools_send_json_error(req, "Failed to add peer");
    }

    ESP_LOGI(TAG, "Peer added: %s (%s)", body.mac, body.label);
    return web_tools_send_json_ok(req);
}
