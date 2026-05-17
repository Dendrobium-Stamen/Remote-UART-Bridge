#include "stdio.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "espnow_manager.h"

#include "message_manager.h"
#include "message_manager_mode.h"

static const char *TAG = "Message Manager Handler - Send Scan Response";

message_manager_error_t message_manager_scan_handler(uint8_t *src_mac, uint8_t *data, size_t data_length)
{
    if (src_mac == NULL || data == NULL || data_length == 0)
    {
        ESP_LOGI(TAG, "Failed to send scan response, src_mac is NULL");
        return MESSAGE_MANAGER_ERROR_SCAN_RESPONSE;
    }

    message_manager_scan_message_t scan_message;
    memcpy(&scan_message, data, data_length);
    espnow_manager_get_label(scan_message.label, ESPNOW_MANAGER_MAX_LABEL_LENGTH);

    lwpkt_write(&message_manager.lwpkt, MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE, &scan_message, sizeof(message_manager_scan_message_t));
    size_t packet_size = lwrb_get_full(&message_manager.send_rb);
    if (packet_size <= 0)
    {
        lwrb_reset(&message_manager.send_rb);
        return MESSAGE_MANAGER_ERROR_SCAN;
    }

    uint8_t *packet_data = malloc(packet_size);
    lwrb_read(&message_manager.send_rb, packet_data, packet_size);

    espnow_manager_temporary_add_peer_mac(src_mac);

    espnow_manager_error_t error = espnow_manager_send_to_mac(src_mac, packet_data, packet_size);
    if (error != ESPNOW_MANAGER_OK)
    {
        free(packet_data);
        espnow_manager_temporary_del_peer_mac(src_mac);
        return MESSAGE_MANAGER_ERROR_SCAN;
    }

    espnow_manager_temporary_del_peer_mac(src_mac);
    free(packet_data);
    return MESSAGE_MANAGER_OK;
}
