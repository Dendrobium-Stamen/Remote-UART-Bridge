#include "stdio.h"

#include "esp_log.h"

#include "message_manager.h"
#include "message_manager_mode.h"

static const char *TAG = "Message Manager Send USB Line Code Change";

bool message_manager_send_usb_line_code_change(uint32_t bit_rate, uint8_t stop_bits, uint8_t parity, uint8_t data_bits)
{
    message_manager_usb_line_coding_changed_message_t usb_line_coding_changed_message = {
        .bit_rate = bit_rate,
        .stop_bits = stop_bits,
        .parity = parity,
        .data_bits = data_bits};
    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED, (uint8_t *)&usb_line_coding_changed_message, sizeof(message_manager_usb_line_coding_changed_message_t));
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);
    if (packet_size <= 0)
        return false;

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
    return true;
}
