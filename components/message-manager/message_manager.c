#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_timer.h"

#include "lwpkt.h"
#include "espnow_manager.h"

#include "message_manager.h"
#include "message_manager_mode.h"
#include "message_manager_handler.h"

static const char *TAG = "Message Manager";

#define MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE 1024 * 10
#define MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE 1024 * 10

uint8_t message_manager_esp_now_data_recv_buffer[MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE];
uint8_t message_manager_esp_now_data_send_buffer[MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE];

message_manager_t message_manager = {};

uint8_t message_manager_espnow_broadcast_mac[ESPNOW_MANAGER_MAC_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void message_manager_receive(espnow_manager_message_t *espnow_manamger_message)
{
    if (espnow_manamger_message == NULL || espnow_manamger_message->data_length == 0)
        return;

    uint8_t *src_mac = espnow_manamger_message->src_mac;
    uint8_t *dest_mac = espnow_manamger_message->dest_mac;
    int8_t rssi = (int8_t)espnow_manamger_message->rssi;
    uint8_t *data = espnow_manamger_message->data;
    size_t data_length = espnow_manamger_message->data_length;

    ESP_LOGD(TAG, "Received data from " MACSTR ", destination " MACSTR ", RSSI: %d, data length: %d", //
             MAC2STR(src_mac), MAC2STR(dest_mac), rssi, espnow_manamger_message->data_length);

    lwrb_write(&message_manager.recv_rb, data, data_length);

    while (lwpkt_read(&message_manager.lwpkt) == lwpktVALID)
    {
        uint8_t *packet_data = (uint8_t *)lwpkt_get_data(&message_manager.lwpkt);
        size_t packet_data_length = lwpkt_get_data_len(&message_manager.lwpkt);
        message_manager_command_t message_manager_command = (message_manager_command_t)lwpkt_get_cmd(&message_manager.lwpkt);

        switch (message_manager_command)
        {
        case MESSAGE_MANAGER_COMMAND_SCAN:
            message_manager_scan_handler(src_mac, packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE:
            message_manager_scan_response_handler(src_mac, rssi, packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_ADD_PEER_DEVICE:
            message_manager_add_peer_device_handler(src_mac, packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA:
            message_manager_usb_to_uart_handler(packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_UART_TO_USB_DATA:
            message_manager_uart_to_usb_handler(packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED:
            message_manager_handle_usb_line_coding_changed_handler(packet_data, packet_data_length);
            break;

        case MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED:
            message_manager_handle_usb_line_state_changed_handler(packet_data, packet_data_length);
            break;

        default:
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

    memset(&message_manager, 0, sizeof(message_manager_t));

    message_manager.usb_to_uart_data_callback = config->usb_to_uart_data_callback;
    message_manager.uart_to_usb_data_callback = config->uart_to_usb_data_callback;
    message_manager.line_coding_changed_baud_callback = config->line_coding_changed_baud_callback;
    message_manager.line_state_changed_callback = config->line_state_changed_callback;

    lwrb_init(&message_manager.recv_rb, message_manager_esp_now_data_recv_buffer, MESSAGE_MANAGER_ESP_NOW_DATA_RECV_MAX_SIZE);
    lwrb_init(&message_manager.send_rb, message_manager_esp_now_data_send_buffer, MESSAGE_MANAGER_ESP_NOW_DATA_SEND_MAX_SIZE);
    lwpkt_init(&message_manager.lwpkt, &message_manager.send_rb, &message_manager.recv_rb);

    lwpkt_reset(&message_manager.lwpkt);
    lwrb_reset(&message_manager.recv_rb);
    lwrb_reset(&message_manager.send_rb);

    return MESSAGE_MANAGER_OK;
}
