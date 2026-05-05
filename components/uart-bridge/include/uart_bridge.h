#ifndef uart_bridge_h
#define uart_bridge_h

#include "stdint.h"

typedef void *uart_bridge_handle_t;
typedef size_t uart_bridge_uart_forward_callback_t(uint8_t *data, size_t data_length);

typedef enum
{
    UART_BRIDGE_OK,
    UART_BRIDGE_ERROR,
    UART_BRIDGE_DELETE_ERROR,
} uart_bridge_error_t;

typedef struct
{
    int uart_tx_gpio_num;
    int uart_rx_gpio_num;

    int baud_rate;

    uart_bridge_uart_forward_callback_t *forward_callback;
} uart_bridge_config_t;

uart_bridge_handle_t uart_bridge_handle_create(uart_bridge_config_t *uart_bridge_config);
uart_bridge_error_t uart_bridge_handle_delete(uart_bridge_handle_t uart_bridge_handle);
uart_bridge_error_t uart_bridge_task_start(uart_bridge_handle_t uart_bridge_handle);

size_t uart_bridge_tx(uint8_t *data, size_t data_length);

bool uart_bridge_reset_baud_rate(int baud_rate);

#endif
