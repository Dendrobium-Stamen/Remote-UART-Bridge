#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <cJSON.h>

static const char *TAG = "http_server";

/* ---- POST /api/scan ---- */
esp_err_t api_scan_handler(httpd_req_t *req)
{
    /* 执行扫描（阻塞约 2 秒） */

    /* 构建 JSON 响应 */
    // cJSON *arr = cJSON_CreateArray();

    // for (int i = 0; i < g_scan_count; i++)
    // {
    //     char mac_str[18];
    //     snprintf(mac_str, sizeof(mac_str),
    //              "%02X:%02X:%02X:%02X:%02X:%02X",
    //              g_scan_results[i].mac[0], g_scan_results[i].mac[1],
    //              g_scan_results[i].mac[2], g_scan_results[i].mac[3],
    //              g_scan_results[i].mac[4], g_scan_results[i].mac[5]);

    //     cJSON *obj = cJSON_CreateObject();
    //     cJSON_AddStringToObject(obj, "mac", mac_str);
    //     cJSON_AddNumberToObject(obj, "rssi", g_scan_results[i].rssi);
    //     cJSON_AddBoolToObject(obj, "is_peer", g_scan_results[i].is_peer);
    //     cJSON_AddItemToArray(arr, obj);
    // }

    // char *json_str = cJSON_PrintUnformatted(arr);
    // cJSON_Delete(arr);

    // httpd_resp_set_type(req, "application/json");
    // esp_err_t err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

    // free(json_str);
    // return err;
    return ESP_OK;
}
