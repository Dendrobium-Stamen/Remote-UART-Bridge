#include "stdio.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_usb_to_uart_handler(uint8_t *packet_data, size_t packet_data_length)
{
    if (packet_data == NULL || packet_data_length == 0)
        return MESSAGE_MANAGER_OK;

    if (message_manager.usb_to_uart_data_callback != NULL)
        message_manager.usb_to_uart_data_callback(packet_data, packet_data_length);

    return MESSAGE_MANAGER_OK;
}
