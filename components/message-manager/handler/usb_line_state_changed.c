#include "stdio.h"

#include "message_manager.h"
#include "message_manager_mode.h"

message_manager_error_t message_manager_handle_usb_line_state_changed_handler(uint8_t *packet_data, size_t packet_data_length)
{
    if (packet_data == NULL || packet_data_length < sizeof(message_manager_usb_line_state_changed_message_t))
        return MESSAGE_MANAGER_ERROR_USB_LINE_STATE_CHANGED;

    message_manager_usb_line_state_changed_message_t message_manager_usb_line_state_changed_message;
    memcpy(&message_manager_usb_line_state_changed_message, packet_data, sizeof(message_manager_usb_line_state_changed_message_t));

    if (message_manager.line_state_changed_callback != NULL)
        message_manager.line_state_changed_callback(message_manager_usb_line_state_changed_message.dtr, message_manager_usb_line_state_changed_message.rts);

    return MESSAGE_MANAGER_OK;
}
