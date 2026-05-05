#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

/* ---- POST /api/peer/del ---- */
static esp_err_t api_peer_del_handler(httpd_req_t *req)
{
    // char buf[256];
    // int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    // if (len <= 0)
    //     return httpd_resp_send_500(req);
    // buf[len] = '\0';

    // cJSON *json = cJSON_Parse(buf);
    // if (!json)
    // {
    //     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    //     return ESP_FAIL;
    // }

    // const char *mac_str = cJSON_GetStringValue(cJSON_GetObjectItem(json, "mac"));
    // if (!mac_str)
    // {
    //     cJSON_Delete(json);
    //     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing MAC");
    //     return ESP_FAIL;
    // }

    // uint8_t mac[6];
    // sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
    //        &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    // esp_err_t err = esp_now_del_peer(mac);
    // cJSON_Delete(json);

    // if (err != ESP_OK)
    // {
    //     httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Peer not found");
    //     return ESP_FAIL;
    // }

    // httpd_resp_set_type(req, "application/json");
    // httpd_resp_sendstr(req, "{\"result\":\"ok\"}");
    // ESP_LOGI(TAG, "Peer deleted: " MACSTR, MAC2STR(mac));
    return ESP_OK;
}
