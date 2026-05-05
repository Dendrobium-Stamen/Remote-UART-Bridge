#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "uart_bridge.h"

static const char *TAG = "Uart bridge";

#define UART_BRIDGE_NUM UART_NUM_1
#define UART_BRIDGE_BUFFER_SIZE (1024 * 10)
#define UART_RX_EVENT_QUEUE_SIZE 10

static uint8_t uart_rx_buffer[UART_BRIDGE_BUFFER_SIZE];

typedef struct
{
    int uart_tx_gpio_num;
    int uart_rx_gpio_num;
    int baud_rate;

    TaskHandle_t uart_rx_task_handle;
    QueueHandle_t uart_event_queue;

    uart_bridge_uart_forward_callback_t *forward_callback;
} uart_bridge_t;

void uart_bridge_forward_task(void *arguments)
{
    uart_bridge_t *uart_bridge = (uart_bridge_t *)arguments;
    uart_event_t uart_event;

    while (1)
    {
        if (xQueueReceive(uart_bridge->uart_event_queue, &uart_event, portMAX_DELAY))
        {
            switch (uart_event.type)
            {
            case UART_DATA:
                if (uart_event.size > 0 && uart_event.size <= UART_BRIDGE_BUFFER_SIZE)
                {
                    int length = uart_read_bytes(UART_BRIDGE_NUM, uart_rx_buffer, uart_event.size, 0);
                    if (length > 0 && uart_bridge->forward_callback)
                    {
                        uart_bridge->forward_callback(uart_rx_buffer, length);
                        // ESP_LOGI(TAG, "UART RX: %d bytes", length);
                    }
                }
                break;

            case UART_FIFO_OVF:
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "UART buffer overflow or full");
                uart_flush_input(UART_BRIDGE_NUM);
                xQueueReset(uart_bridge->uart_event_queue);
                break;

            default:
                break;
            }
        }
    }
}

uart_bridge_handle_t uart_bridge_handle_create(uart_bridge_config_t *config)
{
    if (!config)
        return NULL;

    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));

    uart_config_t uart_config = {
        .baud_rate = config->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    QueueHandle_t event_queue;
    ESP_ERROR_CHECK(uart_driver_install(
        UART_BRIDGE_NUM, UART_BRIDGE_BUFFER_SIZE, UART_BRIDGE_BUFFER_SIZE, UART_RX_EVENT_QUEUE_SIZE, &event_queue, 0));

    // ESP_ERROR_CHECK(uart_set_rx_full_threshold(UART_BRIDGE_NUM, 128));
    // ESP_ERROR_CHECK(uart_set_rx_timeout(UART_BRIDGE_NUM, 10)); // 10个字符的超时/

    ESP_ERROR_CHECK(uart_param_config(UART_BRIDGE_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(
        UART_BRIDGE_NUM,
        config->uart_tx_gpio_num,
        config->uart_rx_gpio_num,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE));

    uart_bridge_t *bridge = calloc(1, sizeof(uart_bridge_t));
    if (!bridge)
    {
        uart_driver_delete(UART_BRIDGE_NUM);
        return NULL;
    }

    bridge->baud_rate = config->baud_rate;
    bridge->uart_tx_gpio_num = config->uart_tx_gpio_num;
    bridge->uart_rx_gpio_num = config->uart_rx_gpio_num;
    bridge->forward_callback = config->forward_callback;
    bridge->uart_event_queue = event_queue;

    return (uart_bridge_handle_t)bridge;
}

uart_bridge_error_t uart_bridge_task_start(uart_bridge_handle_t handle)
{
    uart_bridge_t *uart_bridge = (uart_bridge_t *)handle;
    if (!uart_bridge)
        return UART_BRIDGE_ERROR;

    if (xTaskCreate(uart_bridge_forward_task, "uart_forward_task", 1024 * 4, uart_bridge, 5, &uart_bridge->uart_rx_task_handle) != pdPASS)
        return UART_BRIDGE_ERROR;

    return UART_BRIDGE_OK;
}

size_t uart_bridge_tx(uint8_t *data, size_t data_length)
{
    return uart_write_bytes(UART_BRIDGE_NUM, (const void *)data, data_length);
}

bool uart_bridge_reset_baud_rate(int baud_rate)
{
    uart_set_baudrate(UART_BRIDGE_NUM, baud_rate);
    return true;
}

uart_bridge_error_t uart_bridge_handle_delete(uart_bridge_handle_t handle)
{
    uart_bridge_t *uart_bridge = (uart_bridge_t *)handle;
    if (!uart_bridge)
        return UART_BRIDGE_DELETE_ERROR;

    if (uart_bridge->uart_rx_task_handle)
    {
        vTaskDelete(uart_bridge->uart_rx_task_handle);
    }

    uart_driver_delete(UART_BRIDGE_NUM);

    free(uart_bridge);
    return UART_BRIDGE_OK;
}
