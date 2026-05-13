#include "stdio.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_add_peer_device_handler(uint8_t *src_mac, uint8_t *packet_data, int paket_data_length)
{
    if (src_mac == NULL || packet_data == NULL || paket_data_length == 0)
        return MESSAGE_MANAGER_OK;

    if (paket_data_length < sizeof(message_manager_add_peer_device_message_t))
        return MESSAGE_MANAGER_ERROR_ADD_PEER_DEVICE;

    message_manager_add_peer_device_message_t message_manager_add_peer_device_message;
    memcpy(&message_manager_add_peer_device_message, packet_data, sizeof(message_manager_add_peer_device_message_t));

    espnow_manager_add_peer_mac(src_mac, message_manager_add_peer_device_message.label);

    return MESSAGE_MANAGER_OK;
}
