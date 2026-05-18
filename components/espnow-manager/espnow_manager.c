#include "stdio.h"
#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_now.h"
#include "esp_mac.h"

#include "nvs_manager.h"

#include "espnow_manager.h"
#include "espnow_manager_tools.h"

static const char *TAG = "Espnow Manager";

#define ESPNOW_MANAGER_NVS_KEY "espnow_manager"
#define ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY "espnow_label"

#define ESPNOW_MANAGER_MAX_QUEUE_LENGTH 20

uint8_t espnow_manager_broadcast_mac[ESPNOW_MANAGER_MAC_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t espnow_manager_blank_mac[ESPNOW_MANAGER_MAC_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct
{
    uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];
    bool is_enable;
} espnow_manager_device_t;

typedef struct
{
    espnow_manager_device_t device[ESPNOW_MANAGER_MAX_PEER_DEVICES];
    uint8_t current_device_count;
} espnow_manager_devices_t;

typedef struct
{
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];

    espnow_manager_devices_t *devices;

    espnow_manager_receive_message_callback_t *receive_message_callback;

    QueueHandle_t receive_message_queue;
    TaskHandle_t receive_task_handle;
} espnow_manager_t;

espnow_manager_t espnow_manager = {
    .devices = NULL,
    .receive_message_queue = NULL,
    .receive_task_handle = NULL,
};

void espnow_manager_receive_callback(const esp_now_recv_info_t *esp_now_recv_info, const uint8_t *data, int data_length)
{
    espnow_manager_message_t message;
    memcpy(message.src_mac, esp_now_recv_info->src_addr, ESPNOW_MANAGER_MAC_LEN);
    memcpy(message.dest_mac, esp_now_recv_info->des_addr, ESPNOW_MANAGER_MAC_LEN);
    memcpy(message.data, data, data_length);
    message.rssi = esp_now_recv_info->rx_ctrl->rssi;
    message.data_length = data_length;

    ESP_LOGD(TAG, "Received data from " MACSTR ", destination " MACSTR ", RSSI: %d, data length: %d", MAC2STR(message.src_mac), MAC2STR(message.dest_mac), message.rssi, message.data_length);
    xQueueSend(espnow_manager.receive_message_queue, &message, portMAX_DELAY);
}

espnow_manager_error_t espnow_manager_init(espnow_manager_config_t *config)
{
    if (config == NULL)
        return ESPNOW_MANAGER_ERROR_INIT;

    size_t espnow_manager_devices_size = sizeof(espnow_manager_devices_t);
    espnow_manager_devices_t *espnow_manager_devices = (espnow_manager_devices_t *)malloc(espnow_manager_devices_size);
    memset(espnow_manager_devices, 0, espnow_manager_devices_size);
    memset(&espnow_manager, 0, sizeof(espnow_manager_t));

    if (config->receive_message_callback == NULL)
        ESP_LOGW(TAG, "Receive message callback is null, espnow manager will not receive any message.");

    size_t espnow_manager_nvs_onelabel_langth = ESPNOW_MANAGER_MAX_LABEL_LENGTH;
    if (nvs_manager_get_blob(ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY, espnow_manager.label, &espnow_manager_nvs_onelabel_langth) != NVS_MANAGER_OK)
    {
        ESP_LOGW(TAG, "NVS Label is null, use config label.");
        if (config->label != NULL)
        {
            if (strlen(config->label) > ESPNOW_MANAGER_MAX_LABEL_LENGTH)
            {
                ESP_LOGW(TAG, "Label length is too long, max length is %d, use default label.", ESPNOW_MANAGER_MAX_LABEL_LENGTH);
                uint8_t mac[6];
                esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
                snprintf(espnow_manager.label, sizeof(espnow_manager.label), MACSTR, MAC2STR(mac));
            }
            else
                strncpy(espnow_manager.label, config->label, strlen(config->label));

            ESP_LOGI(TAG, "Use config label : %s", espnow_manager.label);
            nvs_manager_set_blob(ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY, espnow_manager.label, ESPNOW_MANAGER_MAX_LABEL_LENGTH);
        }
        else if (config->label == NULL)
        {
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
            snprintf(espnow_manager.label, sizeof(espnow_manager.label), MACSTR, MAC2STR(mac));
            ESP_LOGI(TAG, "Use default label : %s", espnow_manager.label);
            nvs_manager_set_blob(ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY, espnow_manager.label, ESPNOW_MANAGER_MAX_LABEL_LENGTH);
        }
    }

    espnow_manager.receive_message_callback = config->receive_message_callback;

    if (nvs_manager_get_blob(ESPNOW_MANAGER_NVS_KEY, espnow_manager_devices, &espnow_manager_devices_size) != NVS_MANAGER_OK)
    {
        nvs_manager_erase_all();
        memset(espnow_manager_devices, 0, espnow_manager_devices_size);
        nvs_manager_set_blob(ESPNOW_MANAGER_NVS_KEY, espnow_manager_devices, espnow_manager_devices_size);
        nvs_manager_set_blob(ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY, espnow_manager.label, ESPNOW_MANAGER_MAX_LABEL_LENGTH);
        ESP_LOGW(TAG, "Failed to get espnow manager from nvs, erase all nvs, load default espnow manager.");
    }

    espnow_manager.devices = espnow_manager_devices;
    espnow_manager.receive_message_queue = xQueueCreate(ESPNOW_MANAGER_MAX_QUEUE_LENGTH, sizeof(espnow_manager_message_t));
    espnow_manager.receive_task_handle = NULL;

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_manager_receive_callback));

    espnow_manager_tools_add_peer(espnow_manager_broadcast_mac);
    for (uint8_t i = 0; i < espnow_manager.devices->current_device_count; i++)
        espnow_manager_tools_add_peer(espnow_manager.devices->device[i].mac);

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_deinit()
{
    esp_now_deinit();

    if (espnow_manager.devices != NULL)
        free(espnow_manager.devices);

    if (espnow_manager.receive_message_queue != NULL)
        vQueueDelete(espnow_manager.receive_message_queue);

    espnow_manager.devices = NULL;
    espnow_manager.receive_message_queue = NULL;
    espnow_manager.receive_task_handle = NULL;
    espnow_manager.receive_message_callback = NULL;

    return ESPNOW_MANAGER_OK;
}

void espnow_manager_task(void *arguments)
{
    espnow_manager_message_t message;

    while (1)
    {
        memset(&message, 0, sizeof(espnow_manager_message_t));

        if (xQueueReceive(espnow_manager.receive_message_queue, &message, portMAX_DELAY) == pdTRUE)
        {
            if (espnow_manager.receive_message_callback != NULL)
            {
                espnow_manager.receive_message_callback(&message);
            }
        }
    }
}

espnow_manager_error_t espnow_manager_task_start()
{
    xTaskCreate(espnow_manager_task, "espnow_manager_task", 1024 * 4, NULL, 5, &espnow_manager.receive_task_handle);
    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_task_stop()
{
    if (espnow_manager.receive_task_handle != NULL)
    {
        vTaskDelete(espnow_manager.receive_task_handle);
        espnow_manager.receive_task_handle = NULL;
    }
    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_get_label(char *label, uint8_t label_length)
{
    if (label == NULL)
        return ESPNOW_MANAGER_ERROR_LABEL_GET;

    if (label_length > ESPNOW_MANAGER_MAX_LABEL_LENGTH)
    {
        ESP_LOGW(TAG, "Label length is too long, max length is %d, use default label.", ESPNOW_MANAGER_MAX_LABEL_LENGTH);
        return ESPNOW_MANAGER_ERROR_LABEL_GET;
    }

    memcpy(label, espnow_manager.label, strlen(espnow_manager.label));
    ESP_LOGI(TAG, "espnow_manager_get_label : %s", label);
    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_set_label(char *label, uint8_t label_length)
{
    if (label == NULL)
        return ESPNOW_MANAGER_ERROR_LABEL_SET;

    if (label_length > ESPNOW_MANAGER_MAX_LABEL_LENGTH)
    {
        ESP_LOGW(TAG, "Label length is too long, max length is %d, use default label.", ESPNOW_MANAGER_MAX_LABEL_LENGTH);
        return ESPNOW_MANAGER_ERROR_LABEL_SET;
    }

    memset(espnow_manager.label, 0, ESPNOW_MANAGER_MAX_LABEL_LENGTH);
    memcpy(espnow_manager.label, label, label_length);

    if (nvs_manager_set_blob(ESPNOW_MANAGER_NVS_ONESELF_LABEL_KEY, espnow_manager.label, ESPNOW_MANAGER_MAX_LABEL_LENGTH) != NVS_MANAGER_OK)
        return ESPNOW_MANAGER_ERROR_PEER_ADD;

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_add_peer_mac(uint8_t *mac, char *label)
{
    if (mac == NULL)
        return ESPNOW_MANAGER_ERROR_PEER_ADD;

    if (espnow_manager.devices->current_device_count >= ESPNOW_MANAGER_MAX_PEER_DEVICES)
        return ESPNOW_MANAGER_ERROR_PEER_ADD;

    espnow_manager_error_t error = espnow_manager_tools_add_peer(mac);
    if (error != ESPNOW_MANAGER_OK)
        return error;

    memcpy(espnow_manager.devices->device[espnow_manager.devices->current_device_count].mac, mac, ESPNOW_MANAGER_MAC_LEN);
    if (label != NULL && strlen(label) > 0 && strlen(label) < ESPNOW_MANAGER_MAX_LABEL_LENGTH)
    {
        strncpy(espnow_manager.devices->device[espnow_manager.devices->current_device_count].label, label, strlen(label));
    }
    else
    {
        sprintf(espnow_manager.devices->device[espnow_manager.devices->current_device_count].label, " " MACSTR " ", MAC2STR(mac));
    }

    espnow_manager.devices->device[espnow_manager.devices->current_device_count].is_enable = true;

    espnow_manager.devices->current_device_count++;

    size_t espnow_manager_devices_size = sizeof(espnow_manager_devices_t);
    if (nvs_manager_set_blob(ESPNOW_MANAGER_NVS_KEY, espnow_manager.devices, espnow_manager_devices_size))
        return ESPNOW_MANAGER_ERROR_PEER_ADD;

    ESP_LOGI(TAG, "add peer label : %s", label);

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_del_peer_mac(uint8_t *mac)
{
    if (mac == NULL)
        return ESPNOW_MANAGER_ERROR_PEER_DEL;

    for (int i = 0; i < espnow_manager.devices->current_device_count; i++)
    {
        if (memcmp(espnow_manager.devices->device[i].mac, mac, ESPNOW_MANAGER_MAC_LEN) == 0)
        {
            espnow_manager.devices->current_device_count--;
            memset(espnow_manager.devices->device[i].label, 0, ESPNOW_MANAGER_MAX_LABEL_LENGTH);
            memcpy(espnow_manager.devices->device[i].mac, espnow_manager.devices->device[espnow_manager.devices->current_device_count].mac, ESPNOW_MANAGER_MAC_LEN);
            memcpy(espnow_manager.devices->device[i].label, espnow_manager.devices->device[espnow_manager.devices->current_device_count].label, //
                   strlen(espnow_manager.devices->device[espnow_manager.devices->current_device_count].label) + 1);
            espnow_manager.devices->device[i].is_enable = false;
            break;
        }
    }

    size_t espnow_manager_devices_size = sizeof(espnow_manager_devices_t);
    if (nvs_manager_set_blob(ESPNOW_MANAGER_NVS_KEY, espnow_manager.devices, espnow_manager_devices_size))
        return ESPNOW_MANAGER_ERROR_PEER_DEL;

    if (esp_now_del_peer(mac) != ESP_OK)
    {
        ESP_LOGI(TAG, "del peer label : " MACSTR " error", MAC2STR(mac));
        return ESPNOW_MANAGER_ERROR_PEER_DEL;
    }

    ESP_LOGI(TAG, "del peer label : " MACSTR " success", MAC2STR(mac));
    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_temporary_add_peer_mac(uint8_t *mac)
{
    return espnow_manager_tools_add_peer(mac);
}

espnow_manager_error_t espnow_manager_temporary_del_peer_mac(uint8_t *mac)
{
    if (esp_now_del_peer(mac) != ESP_OK)
        return ESPNOW_MANAGER_ERROR_PEER_DEL;

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_get_peer_mac_count(uint8_t *count, size_t *max_label_length)
{
    if (count == NULL || max_label_length == NULL)
        return ESPNOW_MANAGER_ERROR_PEER_COUNT_FIND;

    *count = espnow_manager.devices->current_device_count;
    *max_label_length = ESPNOW_MANAGER_MAX_LABEL_LENGTH;

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_get_peer_mac(uint8_t index, uint8_t *mac, char *label, bool *is_enable)
{
    if (mac == NULL || label == NULL || is_enable == NULL)
        return ESPNOW_MANAGER_ERROR_PEER_GET;

    if (index >= espnow_manager.devices->current_device_count)
        return ESPNOW_MANAGER_ERROR_PEER_GET;

    memcpy(mac, espnow_manager.devices->device[index].mac, ESPNOW_MANAGER_MAC_LEN);
    strncpy(label, espnow_manager.devices->device[index].label, strlen(espnow_manager.devices->device[index].label) + 1);
    *is_enable = espnow_manager.devices->device[index].is_enable;

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_enable_mac(uint8_t *mac)
{
    if (mac == NULL)
        return ESPNOW_MANAGER_ERROR_ENABLE_MAC;

    for (uint8_t i = 0; i < espnow_manager.devices->current_device_count; i++)
    {
        if (memcmp(espnow_manager.devices->device[i].mac, mac, ESPNOW_MANAGER_MAC_LEN) == 0)
        {
            espnow_manager.devices->device[i].is_enable = true;
            break;
        }
    }

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_disable_mac(uint8_t *mac)
{
    if (mac == NULL)
        return ESPNOW_MANAGER_ERROR_DISABLE_MAC;

    for (uint8_t i = 0; i < espnow_manager.devices->current_device_count; i++)
    {
        if (memcmp(espnow_manager.devices->device[i].mac, mac, ESPNOW_MANAGER_MAC_LEN) == 0)
        {
            espnow_manager.devices->device[i].is_enable = false;
            break;
        }
    }

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_send_to_mac(uint8_t *mac, uint8_t *data, size_t data_length)
{
    if (mac == NULL || data == NULL || data_length <= 0 || data_length >= ESP_NOW_MAX_DATA_LEN_V2)
        return ESPNOW_MANAGER_ERROR_SEND;

    if (esp_now_send(mac, data, data_length) != ESP_OK)
        return ESPNOW_MANAGER_ERROR_SEND;

    return ESPNOW_MANAGER_OK;
}

espnow_manager_error_t espnow_manager_send_to_enable_mac(uint8_t *data, size_t data_length)
{
    if (data == NULL || data_length <= 0 || data_length >= ESP_NOW_MAX_DATA_LEN_V2)
        return ESPNOW_MANAGER_ERROR_SEND;

    for (uint8_t i = 0; i < espnow_manager.devices->current_device_count; i++)
    {
        if (espnow_manager.devices->device[i].is_enable)
        {
            esp_err_t err = esp_now_send(espnow_manager.devices->device[i].mac, data, data_length);
            if (err != ESP_OK)
            {
                ESP_LOGW(TAG, "%s", esp_err_to_name(err));
                return ESPNOW_MANAGER_ERROR_SEND;
            }
        }
    }

    return ESPNOW_MANAGER_OK;
}
