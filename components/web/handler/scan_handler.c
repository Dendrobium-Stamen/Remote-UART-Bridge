#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

/* ---- POST /api/scan ---- */
static esp_err_t api_scan_handler(httpd_req_t *req)
{
    // s_scanned_count = 0;
    // s_scanning = true;

    // /* 广播扫描请求 */
    // uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // uint8_t scan_msg[] = "SCAN";
    // esp_now_send(broadcast, scan_msg, sizeof(scan_msg));

    // /* 等待 2 秒收集响应 */
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // s_scanning = false;

    // /* 返回结果 */
    // cJSON *arr = cJSON_CreateArray();
    // for (int i = 0; i < s_scanned_count; i++)
    // {
    //     cJSON *obj = cJSON_CreateObject();
    //     char mac_str[18];
    //     snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(s_scanned[i].mac));
    //     cJSON_AddStringToObject(obj, "mac", mac_str);
    //     cJSON_AddNumberToObject(obj, "rssi", s_scanned[i].rssi);
    //     cJSON_AddBoolToObject(obj, "is_peer", esp_now_is_peer_exist(s_scanned[i].mac));
    //     cJSON_AddItemToArray(arr, obj);
    // }

    // const char *resp = cJSON_PrintUnformatted(arr);
    // httpd_resp_set_type(req, "application/json");
    // httpd_resp_sendstr(req, resp);
    // cJSON_free((void *)resp);
    // cJSON_Delete(arr);
    return ESP_OK;
}
