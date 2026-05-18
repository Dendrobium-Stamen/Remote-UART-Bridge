#include "stdio.h"

#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_server.h"

#include "web_tools.h"

static const char *TAG = "ota";

esp_err_t ota_upload_handler(httpd_req_t *req)
{
    const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
    if (!part)
        return web_tools_send_json_error(req, "No OTA partition");

    esp_ota_handle_t ota = 0;
    esp_err_t err = esp_ota_begin(part, OTA_WITH_SEQUENTIAL_WRITES, &ota);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        return web_tools_send_json_error(req, "OTA begin failed");
    }

    ESP_LOGI(TAG, "OTA start, partition 0x%" PRIx32 ", content_len=%d",
             part->address, req->content_len);

    char buf[4096];
    int remaining = req->content_len;
    int written = 0;

    while (remaining > 0)
    {
        int to_read = remaining > (int)sizeof(buf) ? (int)sizeof(buf) : remaining;
        int recvd = httpd_req_recv(req, buf, to_read);
        if (recvd <= 0)
        {
            if (recvd == HTTPD_SOCK_ERR_TIMEOUT)
                continue;
            ESP_LOGE(TAG, "Connection lost at %d bytes", written);
            esp_ota_abort(ota);
            return web_tools_send_json_error(req, "Connection lost during upload");
        }

        err = esp_ota_write(ota, buf, recvd);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_ota_write failed at %d: %s", written, esp_err_to_name(err));
            esp_ota_abort(ota);
            return web_tools_send_json_error(req, "OTA write failed");
        }

        written += recvd;
        remaining -= recvd;
    }

    err = esp_ota_end(ota);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
        return web_tools_send_json_error(req, "OTA image verification failed");
    }

    err = esp_ota_set_boot_partition(part);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        return web_tools_send_json_error(req, "Set boot partition failed");
    }

    ESP_LOGI(TAG, "OTA success (%d bytes), rebooting...", written);
    web_tools_send_json_ok(req);
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}
