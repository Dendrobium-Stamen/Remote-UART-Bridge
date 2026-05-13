#include "stdio.h"

#include "esp_log.h"

#include "message_manager.h"
#include "message_manager_mode.h"

static const char *TAG = "Message Manager Send USB Line Code Change";

bool message_manager_send_usb_line_state_change(bool dtr, bool rts)
{
    message_manager_usb_line_state_changed_message_t message_manager_usb_line_state_changed_message = {
        .dtr = dtr,
        .rts = rts};

    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED, (uint8_t *)&message_manager_usb_line_state_changed_message, sizeof(message_manager_usb_line_state_changed_message_t));
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
