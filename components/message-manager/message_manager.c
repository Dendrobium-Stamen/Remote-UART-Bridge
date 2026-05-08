#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_now.h"
#include "esp_mac.h"
#include "esp_timer.h"

#include "lwpkt.h"

#include "message_manager.h"

static const char *TAG = "Message Manager";

#define MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE 1024
#define MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE 1024

uint8_t message_manager_esp_now_data_recv_buffer[MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE];
uint8_t message_manager_esp_now_data_send_buffer[MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE];

typedef enum
{
    MESSAGE_MANAGER_COMMAND_SCAN,
    MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE,
    MESSAGE_MANAGER_COMMAND_PEER,
    MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA,
    MESSAGE_MANAGER_COMMAND_UART_TO_USB_DATA,
    MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED,
    MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED,
} message_manager_command_t;

typedef struct
{
    lwpkt_t lwpkt;
    lwrb_t recv_rb;
    lwrb_t send_rb;

    nvs_store_t *nvs_store;

    message_manager_scan_result_t scan_result;

    message_manager_usb_to_uart_data_callback_t *usb_to_uart_data_callback;
    message_manager_line_coding_changed_baud_callback_t *line_coding_changed_baud_callback;
    message_manager_line_state_changed_callback_t *line_state_changed_callback;
} message_manager_t;

message_manager_t message_manager = {};

static uint8_t message_manager_espnow_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void message_manager_esp_now_recv_callback(const esp_now_recv_info_t *esp_now_recv_info, const uint8_t *data, int data_length)
{
    if (data_length == 0)
        return;

    uint8_t *src_addr = esp_now_recv_info->src_addr;
    uint8_t *des_addr = esp_now_recv_info->des_addr;
    int8_t rssi = (int8_t)esp_now_recv_info->rx_ctrl->rssi;

    ESP_LOGI(TAG, "Received data from " MACSTR ", destination " MACSTR ", RSSI: %d, data length: %d", MAC2STR(src_addr), MAC2STR(des_addr), rssi, data_length);

    lwrb_write(&message_manager.recv_rb, data, data_length);

    while (lwpkt_read(&message_manager.lwpkt) == lwpktVALID)
    {
        size_t data_length = lwpkt_get_data_len(&message_manager.lwpkt);
        uint8_t *pkt_data = (uint8_t *)lwpkt_get_data(&message_manager.lwpkt);
        message_manager_command_t message_manager_command = (message_manager_command_t)lwpkt_get_cmd(&message_manager.lwpkt);

        switch (message_manager_command)
        {
        case MESSAGE_MANAGER_COMMAND_SCAN:
            message_manager_send_scan_response(src_addr, pkt_data, data_length);
            ESP_LOGI(TAG, "Received scan command, sending scan response, mac : " MACSTR " ", MAC2STR(src_addr));
            break;
        case MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE:
            ESP_LOGI(TAG, "Received scan response from " MACSTR ", RSSI: %d, data length: %d", MAC2STR(src_addr), rssi, data_length);
            ESP_LOG_BUFFER_HEX(TAG, pkt_data, data_length);

            if (data_length < sizeof(message_manager.scan_result.timestamp))
                return;

            if (memcmp(&message_manager.scan_result.timestamp, pkt_data, sizeof(message_manager.scan_result.timestamp)) != 0)
            {
                ESP_LOGW(TAG, "Scan response timestamp mismatch");
                return;
            }

            if (message_manager.scan_result.count >= ESP_NOW_MAX_ENCRYPT_PEER_NUM)
            {
                return;
            }

            memcpy(message_manager.scan_result.macs[message_manager.scan_result.count], src_addr, ESP_NOW_ETH_ALEN);
            message_manager.scan_result.rssi[message_manager.scan_result.count] = rssi;
            message_manager.scan_result.count++;

            break;
        case MESSAGE_MANAGER_COMMAND_PEER:
            message_manager_add_peer_mac(src_addr);
            break;
        case MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA:
            if (message_manager.usb_to_uart_data_callback != NULL)
                message_manager.usb_to_uart_data_callback(pkt_data, data_length);

            break;

        case MESSAGE_MANAGER_COMMAND_UART_TO_USB_DATA:
            break;

        case MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED:
            if (data_length == 7)
            {
                uint32_t bit_rate = (pkt_data[0] << 24) | (pkt_data[1] << 16) | (pkt_data[2] << 8) | pkt_data[3];
                uint8_t stop_bits = pkt_data[4];
                uint8_t parity = pkt_data[5];
                uint8_t data_bits = pkt_data[6];

                ESP_LOGI(TAG, "Parsed line code change - Bit Rate: %u, Stop Bits: %u, Parity: %u, Data Bits: %u", bit_rate, stop_bits, parity, data_bits);
                if (message_manager.line_coding_changed_baud_callback != NULL)
                    message_manager.line_coding_changed_baud_callback(bit_rate);

                // uart_bridge_set_baud_rate(bit_rate);
            }

            break;

        case MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED:
            ESP_LOGI(TAG, "Received USB line state change packet, length: %d", data_length);

            uint8_t dtr = data[0];
            uint8_t rts = data[1];

            if (message_manager.line_state_changed_callback != NULL)
                message_manager.line_state_changed_callback(dtr, rts);

            break;
        }
    }

    lwrb_reset(message_manager.lwpkt.tx_rb);
}

message_manager_error_t message_manager_init(message_manager_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Message manager config is NULL");
        return MESSAGE_MANAGER_ERROR_INIT;
    }

    ESP_ERROR_CHECK(esp_now_init());

    message_manager.nvs_store = config->nvs_store;
    message_manager.usb_to_uart_data_callback = config->usb_to_uart_data_callback;
    message_manager.line_coding_changed_baud_callback = config->line_coding_changed_baud_callback;
    message_manager.line_state_changed_callback = config->line_state_changed_callback;

    lwrb_init(&message_manager.recv_rb, message_manager_esp_now_data_recv_buffer, MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE);
    lwrb_init(&message_manager.send_rb, message_manager_esp_now_data_send_buffer, MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE);
    lwpkt_init(&message_manager.lwpkt, &message_manager.send_rb, &message_manager.recv_rb);

    lwpkt_reset(&message_manager.lwpkt);
    lwrb_reset(&message_manager.recv_rb);
    lwrb_reset(&message_manager.send_rb);

    esp_now_peer_info_t esp_now_peer_info = {};

    for (int i = 0; i < message_manager.nvs_store->count; i++)
    {
        memcpy(esp_now_peer_info.peer_addr, message_manager.nvs_store->macs[i], ESP_NOW_ETH_ALEN);
        esp_now_peer_info.channel = 0;
        esp_now_peer_info.ifidx = WIFI_IF_AP;
        esp_now_peer_info.encrypt = false;
        ESP_ERROR_CHECK(esp_now_add_peer(&esp_now_peer_info));
    }

    memset(esp_now_peer_info.peer_addr, 0xFF, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = 0;
    esp_now_peer_info.ifidx = WIFI_IF_AP;
    esp_now_peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&esp_now_peer_info));

    ESP_ERROR_CHECK(esp_now_register_recv_cb(message_manager_esp_now_recv_callback));

    return MESSAGE_MANAGER_OK;
}

message_manager_error_t message_manager_add_peer_mac(uint8_t *peer_mac)
{
    if (nvs_store_compare(message_manager.nvs_store, peer_mac) != NVS_STORE_OK)
        return MESSAGE_MANAGER_ERROR_ADD_PEER;

    if (nvs_store_add(message_manager.nvs_store, peer_mac) != NVS_STORE_OK)
        return MESSAGE_MANAGER_ERROR_ADD_PEER;

    if (nvs_store_save(message_manager.nvs_store) != NVS_STORE_OK)
        return MESSAGE_MANAGER_ERROR_ADD_PEER;

    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = 0;
    esp_now_peer_info.ifidx = WIFI_IF_AP;
    esp_now_peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&esp_now_peer_info));

    return MESSAGE_MANAGER_OK;
}

message_manager_error_t message_manager_delete_peer_mac(uint8_t *peer_mac)
{
    if (nvs_store_delete(message_manager.nvs_store, peer_mac) != NVS_STORE_OK)
        return MESSAGE_MANAGER_ERROR_DELETE_PEER;

    if (nvs_store_save(message_manager.nvs_store) != NVS_STORE_OK)
        return MESSAGE_MANAGER_ERROR_DELETE_PEER;

    esp_now_del_peer(peer_mac);

    return MESSAGE_MANAGER_OK;
}

message_manager_error_t message_manager_get_peer_mac_count(uint8_t *count)
{
    if (count == NULL)
        return MESSAGE_MANAGER_ERROR_GET_PEER_MAC_COUNT;

    *count = message_manager.nvs_store->count;
    return MESSAGE_MANAGER_OK;
}

message_manager_error_t message_manager_get_peer_mac(int index, uint8_t *mac)
{
    if (mac == NULL)
        return MESSAGE_MANAGER_ERROR_GET_PEER_MAC_LIST;

    if (index < 0 || index >= message_manager.nvs_store->count)
        return MESSAGE_MANAGER_ERROR_GET_PEER_MAC_LIST;

    memcpy(mac, message_manager.nvs_store->macs[index], ESP_NOW_ETH_ALEN);
    return MESSAGE_MANAGER_OK;
}

size_t message_manager_send(uint8_t *data, size_t size)
{
    if (data == NULL || size == 0)
        return 0;

    size_t send_data_length = 0;
    while (send_data_length < size)
    {
        size_t chunk_size = size - send_data_length;
        if (chunk_size > LWPKT_CFG_MAX_DATA_LEN)
            chunk_size = LWPKT_CFG_MAX_DATA_LEN;

        // ESP_LOGI(TAG, "Sending chunk, len: %d", chunk_size);

        lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA, data + send_data_length, chunk_size);
        size_t packet_size = lwrb_get_full(&message_manager.send_rb);

        for (int i = 0; i < message_manager.nvs_store->count; i++)
        {
            esp_err_t ret = esp_now_send(message_manager.nvs_store->macs[i], message_manager_esp_now_data_send_buffer, packet_size);
            if (ret != ESP_OK)
            {
                // ESP_LOGE(TAG, "Failed to send data to peer " MACSTR ", error: %d", MAC2STR(message_manager.nvs_store->macs[i]), ret);
                // ESP_LOGE(TAG, "Error sending the data: %s", esp_err_to_name(ret));
                lwrb_reset(&message_manager.send_rb);
            }
        }

        lwrb_reset(&message_manager.send_rb);
        send_data_length += chunk_size;
    }

    return size;
}

message_manager_error_t message_manager_send_scan()
{
    memset(&message_manager.scan_result, 0, sizeof(message_manager.scan_result));
    message_manager.scan_result.timestamp = esp_timer_get_time();

    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_SCAN, &message_manager.scan_result.timestamp, sizeof(message_manager.scan_result.timestamp));
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);

    esp_err_t err = esp_now_send(message_manager_espnow_broadcast_mac, message_manager_esp_now_data_send_buffer, packet_size);
    if (err != ESP_OK)
    {
        lwrb_reset(&message_manager.send_rb);
    }

    lwrb_reset(&message_manager.send_rb);

    return MESSAGE_MANAGER_OK;
}

message_manager_error_t message_manager_send_scan_response(uint8_t *src_mac, uint8_t *data, size_t data_length)
{
    if (src_mac == NULL)
    {
        ESP_LOGI(TAG, "Failed to send scan response, src_mac is NULL");
        return MESSAGE_MANAGER_ERROR_SEND_SCAN_RESPONSE;
    }

    ESP_LOG_BUFFER_HEX(TAG, data, data_length);

    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE, data, data_length);
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);

    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, src_mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = 0;
    esp_now_peer_info.ifidx = WIFI_IF_AP;
    esp_now_peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&esp_now_peer_info));

    esp_err_t err = esp_now_send(src_mac, message_manager_esp_now_data_send_buffer, packet_size);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to send scan response to peer " MACSTR ", error: %s", MAC2STR(src_mac), esp_err_to_name(err));
        lwrb_reset(&message_manager.send_rb);
        esp_now_del_peer(src_mac);
    }

    esp_now_del_peer(src_mac);
    lwrb_reset(&message_manager.send_rb);

    return MESSAGE_MANAGER_OK;
}

message_manager_scan_result_t *message_manager_get_scan_result(uint64_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));

    return &message_manager.scan_result;
}

bool message_manager_send_usb_line_code_change(uint32_t bit_rate, uint8_t stop_bits, uint8_t parity, uint8_t data_bits)
{
    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED, (uint8_t[]){bit_rate >> 24, bit_rate >> 16, bit_rate >> 8, bit_rate, stop_bits, parity, data_bits}, 7);
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);

    for (int i = 0; i < message_manager.nvs_store->count; i++)
    {
        esp_err_t ret = esp_now_send(message_manager.nvs_store->macs[i], message_manager_esp_now_data_send_buffer, packet_size);
        if (ret != ESP_OK)
        {
            // ESP_LOGE(TAG, "Failed to send data to peer " MACSTR ", error: %d", MAC2STR(message_manager.nvs_store->macs[i]), ret);
            // ESP_LOGE(TAG, "Error sending the data: %s", esp_err_to_name(ret));
            lwrb_reset(&message_manager.send_rb);
        }
    }

    lwrb_reset(&message_manager.send_rb);
    return true;
}

bool message_manager_send_usb_line_state_change(bool dtr, bool rts)
{
    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED, (uint8_t[]){dtr, rts}, 2);
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);

    for (int i = 0; i < message_manager.nvs_store->count; i++)
    {
        esp_err_t ret = esp_now_send(message_manager.nvs_store->macs[i], message_manager_esp_now_data_send_buffer, packet_size);
        if (ret != ESP_OK)
        {
            // ESP_LOGE(TAG, "Failed to send data to peer " MACSTR ", error: %d", MAC2STR(message_manager.nvs_store->macs[i]), ret);
            // ESP_LOGE(TAG, "Error sending the data: %s", esp_err_to_name(ret));
            lwrb_reset(&message_manager.send_rb);
        }
    }

    lwrb_reset(&message_manager.send_rb);
    return true;
}
