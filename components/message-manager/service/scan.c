#include "stdio.h"

#include "esp_timer.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_scan()
{
    memset(&message_manager.scan_result, 0, sizeof(message_manager.scan_result));
    message_manager.scan_result.timestamp = esp_timer_get_time();

    message_manager_scan_message_t scan_message;
    memset(&scan_message, 0, sizeof(message_manager_scan_message_t));
    scan_message.timestamp = message_manager.scan_result.timestamp;
    espnow_manager_get_label(scan_message.label, (uint8_t)sizeof(scan_message.label));

    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_SCAN, &scan_message, sizeof(message_manager_scan_message_t));
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);
    if (packet_size <= 0)
        return MESSAGE_MANAGER_ERROR_SCAN;

    uint8_t *packet_data = malloc(packet_size);
    lwrb_read(&message_manager.send_rb, packet_data, packet_size);

    espnow_manager_error_t error = espnow_manager_send_to_mac(message_manager_espnow_broadcast_mac, packet_data, packet_size);
    if (error != ESPNOW_MANAGER_OK)
    {
        free(packet_data);
        return MESSAGE_MANAGER_ERROR_SCAN;
    }

    free(packet_data);
    return MESSAGE_MANAGER_OK;
}
