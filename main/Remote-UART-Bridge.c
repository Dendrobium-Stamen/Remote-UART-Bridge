#include "stdio.h"

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "memory_display.h"
#include "uart_bridge.h"
#include "wifi_manager.h"
#include "message_manager.h"
#include "nvs_store.h"
#include "web.h"

#ifdef CONFIG_REMOTE_UART_ROLE_SERVER
#include "usb_bridge.h"
#endif

#ifdef CONFIG_REMOTE_UART_ROLE_CLIENT

#define REMOATE_SERIAL_PORT_DEVICE_UART_TX_GPIO 4
#define REMOATE_SERIAL_PORT_DEVICE_UART_RX_GPIO 5

#endif

static const char *TAG = "Remote UART Bridge Main";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nvs_store_t nvs_store = {};
    nvs_store_load(&nvs_store);

    wifi_manager_init();

    web_config_t web_config = {};
    web_init(&web_config);

#ifdef CONFIG_REMOTE_UART_ROLE_SERVER
    message_manager_config_t message_manager_config = {
        .nvs_store = &nvs_store,
        .usb_to_uart_data_callback = NULL,
        .line_coding_changed_baud_callback = NULL,
        .line_state_changed_callback = NULL,
    };

    message_manager_init(&message_manager_config);
#endif

#ifdef CONFIG_REMOTE_UART_ROLE_CLIENT
    uart_bridge_config_t uart_bridge_config = {
        .baud_rate = 115200,
        .uart_tx_gpio_num = REMOATE_SERIAL_PORT_DEVICE_UART_TX_GPIO,
        .uart_rx_gpio_num = REMOATE_SERIAL_PORT_DEVICE_UART_RX_GPIO,
        .forward_callback = message_manager_send,
    };

    uart_bridge_handle_t uart_bridge_handle = uart_bridge_handle_create(&uart_bridge_config);

    message_manager_config_t message_manager_config = {
        .nvs_store = &nvs_store,
        .usb_to_uart_data_callback = uart_bridge_tx,
        .line_coding_changed_baud_callback = uart_bridge_set_baud_rate,
        .line_state_changed_callback = uart_bridge_set_line_state,
    };

    message_manager_init(&message_manager_config);

#endif

#ifdef CONFIG_REMOTE_UART_ROLE_SERVER
    usb_bridge_config_t usb_bridge_config = {
        .forward_callback = message_manager_send,
        .line_coding_changed_callback = message_manager_send_usb_line_code_change,
        .line_state_changed_callback = message_manager_send_usb_line_state_change,
    };

    usb_bridge_init(&usb_bridge_config);
    usb_bridge_task_start();
#endif

#ifdef CONFIG_REMOTE_UART_ROLE_CLIENT
    ESP_LOGI(TAG, "Remote UART Bridge Client");
    uart_bridge_task_start(uart_bridge_handle);
#endif

    while (1)
    {
        print_ram_bar();
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
