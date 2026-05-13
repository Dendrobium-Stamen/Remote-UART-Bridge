// #include "stdio.h"

// #include "esp_log.h"
// #include "esp_http_server.h"

// #include "cJSON.h"

// #include "message_manager.h"

// static const char *TAG = "Web peer add handler";

// /* ---- POST /api/peer/add ---- */
// esp_err_t api_peer_add_handler(httpd_req_t *req)
// {
//     char buf[256];
//     int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
//     if (len <= 0)
//         return httpd_resp_send_500(req);
//     buf[len] = '\0';

//     cJSON *json = cJSON_Parse(buf);
//     if (!json)
//     {
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
//         return ESP_FAIL;
//     }

//     const char *mac_str = cJSON_GetStringValue(cJSON_GetObjectItem(json, "mac"));
//     int channel = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "channel"));
//     bool encrypt = cJSON_IsTrue(cJSON_GetObjectItem(json, "encrypt"));

//     if (!mac_str || strlen(mac_str) < 17)
//     {
//         cJSON_Delete(json);
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid MAC");
//         return ESP_FAIL;
//     }

//     /* 解析 MAC */
//     uint8_t mac[6];
//     sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
//            &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

//     ESP_LOG_BUFFER_HEX(TAG, mac, 6);

//     // /* 检查是否已存在 */
//     // if (esp_now_is_peer_exist(mac))
//     // {
//     //     cJSON_Delete(json);
//     //     httpd_resp_send_err(req, HTTPD_409_CONFLICT, "Peer already exists");
//     //     return ESP_FAIL;
//     // }

//     // /* 添加 */
//     // esp_now_peer_info_t peer = {0};
//     // memcpy(peer.peer_addr, mac, 6);
//     // peer.channel = (channel > 0 && channel <= 14) ? channel : 0;
//     // peer.ifidx = WIFI_IF_STA;
//     // peer.encrypt = encrypt;
//     message_manager_add_peer_mac(mac);

//     // esp_err_t err = esp_now_add_peer(&peer);
//     // cJSON_Delete(json);

//     // if (err != ESP_OK)
//     // {
//     //     httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
//     //                         esp_err_to_name(err));
//     //     return ESP_FAIL;
//     // }

//     httpd_resp_set_type(req, "application/json");
//     httpd_resp_sendstr(req, "{\"result\":\"ok\"}");
//     return ESP_OK;
// }
