#ifndef message_manager_h
#define message_manager_h

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#include "espnow_manager.h"

typedef size_t message_manager_usb_to_uart_data_callback_t(uint8_t *data, size_t size);
typedef size_t message_manager_uart_to_usb_data_callback_t(uint8_t *data, size_t size);
typedef bool message_manager_line_coding_changed_baud_callback_t(int baud_rate);
typedef bool message_manager_line_state_changed_callback_t(bool dtr, bool rts);

typedef enum
{
    MESSAGE_MANAGER_OK,
    MESSAGE_MANAGER_ERROR_INIT,
    MESSAGE_MANAGER_ERROR_ADD_PEER_DEVICE,
    MESSAGE_MANAGER_ERROR_DELETE_PEER,
    MESSAGE_MANAGER_ERROR_GET_PEER_MAC_COUNT,
    MESSAGE_MANAGER_ERROR_GET_PEER_MAC_LIST,
    MESSAGE_MANAGER_ERROR_SCAN,
    MESSAGE_MANAGER_ERROR_SCAN_RESPONSE,
    MESSAGE_MANAGER_ERROR_USB_LINE_CODING_CHANGED,
    MESSAGE_MANAGER_ERROR_USB_LINE_STATE_CHANGED,
} message_manager_error_t;

typedef struct
{
    message_manager_usb_to_uart_data_callback_t *usb_to_uart_data_callback;
    message_manager_uart_to_usb_data_callback_t *uart_to_usb_data_callback;
    message_manager_line_coding_changed_baud_callback_t *line_coding_changed_baud_callback;
    message_manager_line_state_changed_callback_t *line_state_changed_callback;
} message_manager_config_t;

typedef struct
{
    uint8_t mac[ESPNOW_MANAGER_MAC_LEN];
    char label[ESPNOW_MANAGER_MAX_LABEL_LENGTH];
    int rssi;
} message_manager_scan_result_device_t;

typedef struct
{
    message_manager_scan_result_device_t devices[ESPNOW_MANAGER_MAX_PEER_DEVICES];
    size_t device_count;
    int64_t timestamp;
} message_manager_scan_result_t;

message_manager_error_t message_manager_init(message_manager_config_t *config);
message_manager_error_t message_manager_deinit();

void message_manager_receive(espnow_manager_message_t *espnow_manamger_message);

message_manager_error_t message_manager_scan();

message_manager_scan_result_t *message_manager_get_scan_result(uint64_t wait_ms);

size_t message_manager_send_data_usb_to_uart(uint8_t *data, size_t data_length);
size_t message_manager_send_data_uart_to_usb(uint8_t *data, size_t data_length);

bool message_manager_send_usb_line_code_change(uint32_t bit_rate, uint8_t stop_bits, uint8_t parity, uint8_t data_bits);
bool message_manager_send_usb_line_state_change(bool dtr, bool rts);

#endif
