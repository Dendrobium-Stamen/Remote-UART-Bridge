#include "stdio.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_handle_usb_line_coding_changed_handler(uint8_t *packet_data, size_t packet_data_length)
{
    if (packet_data == NULL || packet_data_length < sizeof(message_manager_usb_line_coding_changed_message_t))
        return MESSAGE_MANAGER_ERROR_USB_LINE_CODING_CHANGED;

    message_manager_usb_line_coding_changed_message_t usb_line_coding_changed_message;
    memcpy(&usb_line_coding_changed_message, packet_data, sizeof(message_manager_usb_line_coding_changed_message_t));

    if (message_manager.line_coding_changed_baud_callback != NULL)
        message_manager.line_coding_changed_baud_callback(usb_line_coding_changed_message.bit_rate);

    return MESSAGE_MANAGER_OK;
}
