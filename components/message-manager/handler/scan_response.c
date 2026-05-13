#include "stdio.h"

#include "espnow_manager.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_scan_response_handler(uint8_t *src_mac, int rssi, uint8_t *packet_data, int paket_data_length)
{
    if (paket_data_length < sizeof(message_manager_scan_message_t))
        return MESSAGE_MANAGER_ERROR_SCAN_RESPONSE;

    message_manager_scan_message_t scan_message;
    memcpy(&scan_message, packet_data, sizeof(message_manager_scan_message_t));

    if (memcmp(&message_manager.scan_result.timestamp, &scan_message.timestamp, sizeof(message_manager.scan_result.timestamp)) != 0)
        return MESSAGE_MANAGER_ERROR_SCAN_RESPONSE;

    if (message_manager.scan_result.device_count >= ESPNOW_MANAGER_MAX_PEER_DEVICES)
        return MESSAGE_MANAGER_ERROR_SCAN_RESPONSE;

    memcpy(message_manager.scan_result.devices[message_manager.scan_result.device_count].mac, src_mac, ESPNOW_MANAGER_MAC_LEN);
    memcpy(&message_manager.scan_result.devices[message_manager.scan_result.device_count].label, scan_message.label, strlen(scan_message.label));
    message_manager.scan_result.devices[message_manager.scan_result.device_count].rssi = rssi;
    message_manager.scan_result.device_count++;

    return MESSAGE_MANAGER_OK;
}
