#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "message_manager.h"

static const char *TAG = "http_server";

/* ---- POST /api/scan ---- */
esp_err_t api_scan_handler(httpd_req_t *req)
{
    /* 执行扫描（阻塞约 2 秒） */
    message_manager_error_t error = message_manager_send_scan();
    if (error != MESSAGE_MANAGER_OK)
    {
        ESP_LOGE(TAG, "Failed to send scan request: %d", error);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send scan request");
        return ESP_FAIL;
    }

    message_manager_scan_result_t *scan_result = message_manager_get_scan_result(100);
    if (scan_result == NULL)
    {
        ESP_LOGE(TAG, "Failed to get scan result");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get scan result");
        return ESP_FAIL;
    }

    if (scan_result->count == 0)
    {
        ESP_LOGI(TAG, "No devices found");
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "No devices found");
        return ESP_FAIL;
    }

    /* 构建 JSON 响应 */
    cJSON *arr = cJSON_CreateArray();

    for (int i = 0; i < scan_result->count; i++)
    {
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 scan_result->macs[i][0], scan_result->macs[i][1], scan_result->macs[i][2],
                 scan_result->macs[i][3], scan_result->macs[i][4], scan_result->macs[i][5]);

        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "mac", mac_str);
        cJSON_AddNumberToObject(obj, "rssi", scan_result->rssi[i]);
        cJSON_AddItemToArray(arr, obj);
    }

    char *json_str = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);

    httpd_resp_set_type(req, "application/json");
    esp_err_t err = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

    free(json_str);
    return err;
}
