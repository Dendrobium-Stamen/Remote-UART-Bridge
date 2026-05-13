#ifndef message_manager_handler_h
#define message_manager_handler_h

#include "message_manager.h"

message_manager_error_t
message_manager_scan_handler(uint8_t *src_mac, uint8_t *data, size_t data_length);
message_manager_error_t message_manager_scan_response_handler(uint8_t *src_mac, int rssi, uint8_t *packet_data, int paket_data_length);
message_manager_error_t message_manager_add_peer_device_handler(uint8_t *src_mac, uint8_t *packet_data, int paket_data_length);
message_manager_error_t message_manager_usb_to_uart_handler(uint8_t *packet_data, size_t packet_data_length);
message_manager_error_t message_manager_uart_to_usb_handler(uint8_t *packet_data, size_t packet_data_length);
message_manager_error_t message_manager_handle_usb_line_coding_changed_handler(uint8_t *packet_data, size_t packet_data_length);
message_manager_error_t message_manager_handle_usb_line_state_changed_handler(uint8_t *packet_data, size_t packet_data_length);

#endif
