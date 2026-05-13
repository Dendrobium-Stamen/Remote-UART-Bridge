// #include "stdio.h"

// #include "esp_log.h"
// #include "esp_http_server.h"

// #include "cJSON.h"

// #include "message_manager.h"

// static const char *TAG = "Web peer del handler";

// /* ---- POST /api/peer/del ---- */
// esp_err_t api_peer_del_handler(httpd_req_t *req)
// {
//     /* ── 读取 POST body ── */
//     int total = req->content_len;
//     if (total <= 0 || total > 512)
//     {
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad request body");
//         return ESP_FAIL;
//     }

//     char *buf = calloc(1, total + 1);
//     if (!buf)
//     {
//         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory");
//         return ESP_FAIL;
//     }

//     int received = 0;
//     while (received < total)
//     {
//         int ret = httpd_req_recv(req, buf + received, total - received);
//         if (ret <= 0)
//         {
//             free(buf);
//             httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Recv failed");
//             return ESP_FAIL;
//         }
//         received += ret;
//     }

//     /* ── 解析 JSON ── */
//     cJSON *root = cJSON_Parse(buf);
//     free(buf);
//     if (!root)
//     {
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
//         return ESP_FAIL;
//     }

//     cJSON *j_mac = cJSON_GetObjectItem(root, "mac");
//     if (!cJSON_IsString(j_mac) || !j_mac->valuestring)
//     {
//         cJSON_Delete(root);
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'mac'");
//         return ESP_FAIL;
//     }

//     /* ── 解析 MAC 地址 AA:BB:CC:DD:EE:FF → uint8_t[6] ── */
//     uint8_t mac[6];
//     unsigned m[6];
//     if (strlen(j_mac->valuestring) != 17 ||
//         sscanf(j_mac->valuestring, "%02x:%02x:%02x:%02x:%02x:%02x",
//                &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) != 6)
//     {
//         cJSON_Delete(root);
//         httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid MAC format");
//         return ESP_FAIL;
//     }
//     for (int i = 0; i < 6; i++)
//         mac[i] = (uint8_t)m[i];

//     /* ── 从 ESP-NOW 底层删除 peer ── */
//     message_manager_error_t message_manager_error = message_manager_delete_peer_mac(mac);

//     if (message_manager_error != MESSAGE_MANAGER_OK)
//     {
//         ESP_LOGW(TAG, "message_manager_delete_peer_mac failed: %d", message_manager_error);
//         cJSON_Delete(root);
//         httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Peer not found");
//         return ESP_FAIL;
//     }

//     ESP_LOGI(TAG, "Peer removed: %s", j_mac->valuestring);
//     cJSON_Delete(root);

//     /* ── 返回成功 ── */
//     httpd_resp_set_type(req, "application/json");
//     httpd_resp_set_status(req, "200 OK");
//     return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
// }
