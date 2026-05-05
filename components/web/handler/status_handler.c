#include "stdio.h"

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "message_manager.h"

esp_err_t api_status_handler(httpd_req_t *req)
{
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    uint8_t peer_count = 0;
    message_manager_get_peer_mac_count(&peer_count);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "local_mac", mac_str);
    cJSON_AddNumberToObject(root, "peer_count", peer_count);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    esp_err_t err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to send response");
        return ESP_OK;
    }

    free(json_str);

    return ESP_OK;
}
