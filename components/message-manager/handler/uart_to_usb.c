#include "stdio.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_uart_to_usb_handler(uint8_t *packet_data, size_t packet_data_length)
{
    if (packet_data == NULL || packet_data_length == 0)
        return MESSAGE_MANAGER_OK;

    if (message_manager.uart_to_usb_data_callback != NULL)
        message_manager.uart_to_usb_data_callback(packet_data, packet_data_length);

    return MESSAGE_MANAGER_OK;
}
