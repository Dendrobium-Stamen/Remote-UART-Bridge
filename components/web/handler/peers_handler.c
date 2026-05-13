// #include "stdio.h"

// #include "esp_log.h"
// #include "esp_http_server.h"

// #include "cJSON.h"

// #include "message_manager.h"

// esp_err_t api_peers_handler(httpd_req_t *req)
// {
//     cJSON *peers = cJSON_CreateArray();

//     uint8_t peer_count = 0;
//     message_manager_get_peer_mac_count(&peer_count);

//     for (int i = 0; i < peer_count; i++)
//     {
//         uint8_t mac[6];
//         message_manager_get_peer_mac(i, mac);

//         char mac_str[18];
//         snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
//                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

//         cJSON *obj = cJSON_CreateObject();
//         cJSON_AddStringToObject(obj, "mac", mac_str);
//         cJSON_AddItemToArray(peers, obj);
//     }

//     char *json_str = cJSON_PrintUnformatted(peers);
//     cJSON_Delete(peers);

//     httpd_resp_set_type(req, "application/json");
//     esp_err_t err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

//     free(json_str);

//     return err;
// }
