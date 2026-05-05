#ifndef usb_bridge_h
#define usb_bridge_h

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

typedef size_t usb_bridge_forward_callback_t(uint8_t *rx_buffer, size_t rx_buffer_size);
typedef bool usb_bridge_line_coding_changed_callback_t(uint32_t bit_rate, uint8_t stop_bits, uint8_t parity, uint8_t data_bits);
typedef bool usb_bridge_line_state_changed_callback_t(bool dtr, bool rts);

typedef enum
{
    USB_BRIDGE_OK,
    USB_BRIDGE_INIT_ERROR,
    USB_BRIDGE_READ_RINGBUFFER_ERROR,
    USB_BRIDGE_RX_ERROR,
    USB_BRIDGE_LINE_CODING_CHANGE_RECV_ERROR,
    USB_BRIDGE_LINE_STATE_CHANGE_RECV_ERROR,
    USB_BRIDGE_DELETE_ERROR,
} usb_bridge_error_t;

typedef struct
{
    uint32_t bit_rate;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t data_bits;
} usb_bridge_line_coding_t;

typedef struct
{
    bool dtr;
    bool rts;
} usb_bridge_line_state_changed_data_t;

typedef struct
{
    usb_bridge_forward_callback_t *forward_callback;
    usb_bridge_line_coding_changed_callback_t *line_coding_changed_callback;
    usb_bridge_line_state_changed_callback_t *line_state_changed_callback;
} usb_bridge_config_t;

usb_bridge_error_t usb_bridge_init(usb_bridge_config_t *usb_bridge_config);
usb_bridge_error_t usb_bridge_task_start();

size_t usb_bridge_tx(uint8_t *tx_buffer, size_t tx_buffer_size);
usb_bridge_error_t usb_bridge_rx(uint8_t *rx_buffer, size_t rx_buffer_size, size_t *rx_data_size);
usb_bridge_error_t usb_bridge_line_coding_change_recv(usb_bridge_line_coding_t *usb_bridge_line_coding);
usb_bridge_error_t usb_bridge_line_state_change_recv(usb_bridge_line_state_changed_data_t *usb_bridge_line_state_changed_data);
usb_bridge_error_t usb_bridge_deinit();

#endif
