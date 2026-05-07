#ifndef message_manager_h
#define message_manager_h

#include "stdio.h"
#include "stdint.h"

#include "esp_now.h"

#include "nvs_store.h"

typedef size_t message_manager_usb_to_uart_data_callback_t(uint8_t *data, size_t size);
typedef bool message_manager_line_coding_changed_baud_callback_t(int baud_rate);
typedef bool message_manager_line_state_changed_callback_t(bool dtr, bool rts);

typedef enum
{
    MESSAGE_MANAGER_OK,
    MESSAGE_MANAGER_ERROR_INIT,
    MESSAGE_MANAGER_ERROR_ADD_PEER,
    MESSAGE_MANAGER_ERROR_REMOVE_PEER,
    MESSAGE_MANAGER_ERROR_GET_PEER_MAC_COUNT,
    MESSAGE_MANAGER_ERROR_GET_PEER_MAC_LIST,
    MESSAGE_MANAGER_ERROR_SEND_SCAN_RESPONSE,
} message_manager_error_t;

typedef struct
{
    nvs_store_t *nvs_store;

    message_manager_usb_to_uart_data_callback_t *usb_to_uart_data_callback;
    message_manager_line_coding_changed_baud_callback_t *line_coding_changed_baud_callback;
    message_manager_line_state_changed_callback_t *line_state_changed_callback;
} message_manager_config_t;

typedef struct
{
    uint8_t macs[ESP_NOW_MAX_ENCRYPT_PEER_NUM][6];
    int8_t rssi[ESP_NOW_MAX_ENCRYPT_PEER_NUM];
    uint8_t count;
    int64_t timestamp;
} message_manager_scan_result_t;

message_manager_error_t message_manager_init(message_manager_config_t *config);
size_t message_manager_send(uint8_t *data, size_t size);
message_manager_error_t message_manager_add_peer_mac(uint8_t *peer_mac);
message_manager_error_t message_manager_remove_peer_mac(uint8_t *peer_mac);
message_manager_error_t message_manager_get_peer_mac_count(uint8_t *count);
message_manager_error_t message_manager_get_peer_mac(int index, uint8_t *mac);
message_manager_error_t message_manager_send_scan();
message_manager_error_t message_manager_send_scan_response(uint8_t *src_mac);
message_manager_scan_result_t *message_manager_get_scan_result(uint64_t wait_ms);

message_manager_error_t message_manager_reset_peer_mac();
message_manager_error_t message_manager_deinit();

bool message_manager_send_usb_line_code_change(uint32_t bit_rate, uint8_t stop_bits, uint8_t parity, uint8_t data_bits);
bool message_manager_send_usb_line_state_change(bool dtr, bool rts);

#endif
