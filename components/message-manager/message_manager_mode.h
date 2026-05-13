#ifndef message_manager_mode_h
#define message_manager_mode_h

#include "lwpkt.h"

#include "message_manager.h"

extern uint8_t message_manager_espnow_broadcast_mac[ESPNOW_MANAGER_MAC_LEN];

typedef struct
{
    int64_t timestamp;
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];
} message_manager_scan_message_t;

typedef struct
{
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];
} message_manager_add_peer_device_message_t;

typedef struct
{
    uint32_t bit_rate;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t data_bits;
} message_manager_usb_line_coding_changed_message_t;

typedef struct
{
    bool dtr;
    bool rts;
} message_manager_usb_line_state_changed_message_t;

typedef enum
{
    MESSAGE_MANAGER_COMMAND_SCAN,
    MESSAGE_MANAGER_COMMAND_SCAN_RESPONSE,
    MESSAGE_MANAGER_COMMAND_ADD_PEER_DEVICE,
    MESSAGE_MANAGER_COMMAND_USB_TO_UART_DATA,
    MESSAGE_MANAGER_COMMAND_UART_TO_USB_DATA,
    MESSAGE_MANAGER_COMMAND_USB_LINE_CODING_CHANGED,
    MESSAGE_MANAGER_COMMAND_USB_LINE_STATE_CHANGED,
} message_manager_command_t;

typedef struct
{
    lwpkt_t lwpkt;
    lwrb_t recv_rb;
    lwrb_t send_rb;

    message_manager_scan_result_t scan_result;

    message_manager_usb_to_uart_data_callback_t *usb_to_uart_data_callback;
    message_manager_uart_to_usb_data_callback_t *uart_to_usb_data_callback;
    message_manager_line_coding_changed_baud_callback_t *line_coding_changed_baud_callback;
    message_manager_line_state_changed_callback_t *line_state_changed_callback;
} message_manager_t;

extern message_manager_t message_manager;

#endif
