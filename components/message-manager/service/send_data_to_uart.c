#include "stdio.h"

#include "esp_log.h"

#include "message_manager.h"
#include "message_manager_mode.h"

static const char *TAG = "Message Manager Send Data to UART";

size_t message_manager_send_data_usb_to_uart(uint8_t *data, size_t data_length)
{
    if (data == NULL || data_length == 0)
        return 0;

    size_t send_data_length = 0;
    while (send_data_length < data_length)
    {
        size_t chunk_size = data_length - send_data_length;
        if (chunk_size > LWPKT_CFG_MAX_DATA_LEN)
            chunk_size = LWPKT_CFG_MAX_DATA_LEN;

        // ESP_LOGI(TAG, "Sending chunk, len: %d", chunk_size);

        lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA, data + send_data_length, chunk_size);
        size_t packet_size = lwrb_get_full(&message_manager.send_rb);
        if (packet_size <= 0)
            return 0;

        uint8_t *packet_data = malloc(packet_size);
        lwrb_read(&message_manager.send_rb, packet_data, packet_size);

        espnow_manager_error_t error = espnow_manager_send_to_enable_mac(packet_data, packet_size);
        if (error != ESPNOW_MANAGER_OK)
        {
            lwrb_reset(&message_manager.send_rb);
            ESP_LOGW(TAG, "Failed to send data to peer enable mac, error: %d", error);
            return 0;
        }

        lwrb_reset(&message_manager.send_rb);
        send_data_length += chunk_size;
    }

    return data_length;
}
