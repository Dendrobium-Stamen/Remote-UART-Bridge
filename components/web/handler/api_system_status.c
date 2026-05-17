#include "stdio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_http_server.h"

#include "cJSON.h"

#include "espstate_monitor.h"
#include "espnow_manager.h"

#include "web_tools.h"

static void format_uptime(int64_t us, char *out, size_t len)
{
        int sec = (int)(us / 1000000);
        snprintf(out, len, "%02d:%02d:%02d", sec / 3600, (sec % 3600) / 60, sec % 60);
}

esp_err_t system_status_handler(httpd_req_t *req)
{
        uint32_t heap = esp_get_free_heap_size() / 1024;

        char uptime[16];
        format_uptime(esp_timer_get_time(), uptime, sizeof(uptime));

        float temp = espstate_monitor_chip_temperature_read();

        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        char mac_str[WEB_TOOLS_MAC_TO_STR_LENGTH];
        web_tools_mac_to_str(mac, mac_str);

        char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH] = {0};
        espnow_manager_get_label(label, sizeof(label));

        const char *chip_name =
#if CONFIG_IDF_TARGET_ESP32S3
            "ESP32-S3";
#elif CONFIG_IDF_TARGET_ESP32C3
            "ESP32-C3";
#endif

        cJSON *root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "heap", heap);
        cJSON_AddStringToObject(root, "uptime", uptime);
        if (temp > -100.0f)
                cJSON_AddNumberToObject(root, "temp", temp);
        cJSON_AddStringToObject(root, "sdk", esp_get_idf_version());
        cJSON_AddStringToObject(root, "chip", chip_name);
        cJSON_AddStringToObject(root, "mac", mac_str);
        cJSON_AddStringToObject(root, "label", label);

        esp_err_t ret = web_tools_send_json_resp(req, root);
        cJSON_Delete(root);
        return ret;
}
