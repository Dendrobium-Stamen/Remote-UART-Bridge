#include "stdio.h"
#include "string.h"

#include "esp_err.h"
#include "esp_now.h"

#include "espnow_manager.h"

espnow_manager_error_t espnow_manager_tools_add_peer(uint8_t *mac)
{
    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = 0;
    esp_now_peer_info.ifidx = WIFI_IF_AP;
    esp_now_peer_info.encrypt = false;
    esp_err_t esp_error = esp_now_add_peer(&esp_now_peer_info);
    if (esp_error != ESP_OK)
    {
        if (esp_error == ESP_ERR_ESPNOW_EXIST)
            return ESPNOW_MANAGER_ERROR_PEER_EXISTS;
        if (esp_error == ESP_ERR_ESPNOW_FULL)
            return ESPNOW_MANAGER_ERROR_PEER_FULL;

        return ESPNOW_MANAGER_ERROR_PEER_ADD;
    }

    return ESPNOW_MANAGER_OK;
}
