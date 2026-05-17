#include "stdio.h"

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "nvs_manager.h"
#include "espstate_monitor.h"
#include "memory_display.h"
#include "espnow_manager.h"
#include "uart_bridge.h"
#include "wifi_manager.h"
#include "message_manager.h"
#include "web.h"
#include "auto_download.h"

#ifdef CONFIG_REMOTE_UART_ROLE_SERVER

#include "usb_bridge.h"

#define REMOTE_UART_BRIDGE_NVS_NAMESPACE "server device"

#endif

static const char *TAG = "Remote UART Bridge Main";

#ifdef CONFIG_REMOTE_UART_ROLE_CLIENT

#define REMOTE_UART_BRIDGE_NVS_NAMESPACE "server client"

#define AUTO_DOWNLOAD_DTS_GPIO_NUM 0
#define AUTO_DOWNLOAD_RTS_GPIO_NUM 1

#define REMOATE_SERIAL_PORT_DEVICE_UART_TX_GPIO 4
#define REMOATE_SERIAL_PORT_DEVICE_UART_RX_GPIO 5

#endif

void app_main(void)
{
    nvs_manager_init(REMOTE_UART_BRIDGE_NVS_NAMESPACE);

    espstate_monitor_chip_temperature_init();

    wifi_manager_init(NULL);

    web_config_t web_config = {};
    web_init(&web_config);

    espnow_manager_config_t espnow_manager_config = {
        .label = "I'm giraffe",
        .receive_message_callback = message_manager_receive,
    };

    espnow_manager_init(&espnow_manager_config);

#ifdef CONFIG_REMOTE_UART_ROLE_SERVER
    usb_bridge_config_t usb_bridge_config = {
        .forward_callback = message_manager_send_data_usb_to_uart,
        .line_coding_changed_callback = message_manager_send_usb_line_code_change,
        .line_state_changed_callback = message_manager_send_usb_line_state_change,
    };

    usb_bridge_init(&usb_bridge_config);

    message_manager_config_t message_manager_config = {
        .usb_to_uart_data_callback = NULL,
        .uart_to_usb_data_callback = usb_bridge_tx,
        .line_coding_changed_baud_callback = NULL,
        .line_state_changed_callback = NULL,
    };

    message_manager_init(&message_manager_config);

    espnow_manager_task_start();
    usb_bridge_task_start();

#endif

#ifdef CONFIG_REMOTE_UART_ROLE_CLIENT
    auto_download_config_t auto_download_config = {
        .dtr_gpio_num = AUTO_DOWNLOAD_DTS_GPIO_NUM,
        .rts_gpio_num = AUTO_DOWNLOAD_RTS_GPIO_NUM,
    };

    auto_download_init(&auto_download_config);

    uart_bridge_config_t uart_bridge_config = {
        .baud_rate = 115200,
        .uart_tx_gpio_num = REMOATE_SERIAL_PORT_DEVICE_UART_TX_GPIO,
        .uart_rx_gpio_num = REMOATE_SERIAL_PORT_DEVICE_UART_RX_GPIO,
        .forward_callback = message_manager_send_data_uart_to_usb,
    };

    uart_bridge_handle_t uart_bridge_handle = uart_bridge_handle_create(&uart_bridge_config);

    message_manager_config_t message_manager_config = {
        .usb_to_uart_data_callback = uart_bridge_tx,
        .uart_to_usb_data_callback = NULL,
        .line_coding_changed_baud_callback = uart_bridge_reset_baud_rate,
        .line_state_changed_callback = auto_download_set_gpio_level,
    };

    message_manager_init(&message_manager_config);

    ESP_LOGI(TAG, "Remote UART Bridge Client");
    espnow_manager_task_start();
    uart_bridge_task_start(uart_bridge_handle);

#endif

    while (1)
    {
        print_ram_bar();
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
