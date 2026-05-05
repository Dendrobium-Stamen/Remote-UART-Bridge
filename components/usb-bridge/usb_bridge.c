#include "stdio.h"
#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_err.h"

#include "tinyusb.h"
#include "vfs_tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_default_config.h"

#include "usb_bridge.h"
#include "usb_bridge_tools.h"

static const char *TAG = "Custom cdcacm usb";

#define CFG_BAUD_RATE(b) (b)
#define CFG_STOP_BITS(s) (((s) == 2) ? UART_STOP_BITS_2 : (((s) == 1) ? UART_STOP_BITS_1_5 : UART_STOP_BITS_1))
#define CFG_PARITY(p) (((p) == 2) ? UART_PARITY_EVEN : (((p) == 1) ? UART_PARITY_ODD : UART_PARITY_DISABLE))
#define CFG_DATA_BITS(b) (((b) == 5) ? UART_DATA_5_BITS : (((b) == 6) ? UART_DATA_6_BITS : (((b) == 7) ? UART_DATA_7_BITS : UART_DATA_8_BITS)))

#define STR_STOP_BITS(s) (((s) == 2) ? "UART_STOP_BITS_2" : (((s) == 1) ? "UART_STOP_BITS_1_5" : "UART_STOP_BITS_1"))
#define STR_PARITY(p) (((p) == 2) ? "UART_PARITY_EVEN" : (((p) == 1) ? "UART_PARITY_ODD" : "UART_PARITY_DISABLE"))
#define STR_DATA_BITS(b) (((b) == 5) ? "UART_DATA_5_BITS" : (((b) == 6) ? "UART_DATA_6_BITS" : (((b) == 7) ? "UART_DATA_7_BITS" : "UART_DATA_8_BITS")))

#define USB_BRIDGE_WRITE_BUFFER_SIZE 1024 * 10
#define USB_BRIDGE_READ_BUFFER_SIZE 1024 * 10

uint8_t usb_bridge_write_buffer[USB_BRIDGE_WRITE_BUFFER_SIZE] = {0};
uint8_t usb_bridge_read_buffer[USB_BRIDGE_READ_BUFFER_SIZE] = {0};

static const tusb_desc_device_t cdc_device_descriptor = {
    .bLength = sizeof(cdc_device_descriptor),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = TINYUSB_ESPRESSIF_VID,
    .idProduct = 0x4002,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01};

typedef struct
{
    TaskHandle_t tx_data_task;
    TaskHandle_t rx_data_task;

    TaskHandle_t line_coding_changed_task;
    TaskHandle_t line_state_changed_task;

    RingbufHandle_t usb_tx_ringbuffer;
    RingbufHandle_t usb_rx_forward_ringbuffer;

    QueueHandle_t line_coding_queue;
    QueueHandle_t line_state_changed_data_queue;

    usb_bridge_line_coding_t last_line_coding;
    usb_bridge_line_state_changed_data_t last_line_state_changed_data;

    usb_bridge_forward_callback_t *forward_callback;
    usb_bridge_line_coding_changed_callback_t *line_coding_changed_callback;
    usb_bridge_line_state_changed_callback_t *line_state_changed_callback;
} usb_bridge_t;

usb_bridge_t usb_bridge = {
    .tx_data_task = NULL,
    .rx_data_task = NULL,
    .line_coding_changed_task = NULL,
    .line_state_changed_task = NULL,
    .usb_tx_ringbuffer = NULL,
    .usb_rx_forward_ringbuffer = NULL,
    .line_coding_queue = NULL,
    .line_state_changed_data_queue = NULL,
    .last_line_coding.bit_rate = 115200,
    .last_line_coding.stop_bits = 0,
    .last_line_coding.parity = 0,
    .last_line_coding.data_bits = 0,
    .last_line_state_changed_data.dtr = false,
    .last_line_state_changed_data.rts = false,
    .forward_callback = NULL,
    .line_coding_changed_callback = NULL,
    .line_state_changed_callback = NULL,
};

static void tinyusb_cdc_line_coding_changed_callback(int itf, cdcacm_event_t *event)
{
    uint32_t bit_rate = event->line_coding_changed_data.p_line_coding->bit_rate;
    uint8_t stop_bits = event->line_coding_changed_data.p_line_coding->stop_bits;
    uint8_t parity = event->line_coding_changed_data.p_line_coding->parity;
    uint8_t data_bits = event->line_coding_changed_data.p_line_coding->data_bits;

    bool line_coding_is_change = false;

    if (usb_bridge.last_line_coding.bit_rate != bit_rate)
    {
        usb_bridge.last_line_coding.bit_rate = bit_rate;
        line_coding_is_change = true;
    }

    if (usb_bridge.last_line_coding.stop_bits != stop_bits)
    {
        usb_bridge.last_line_coding.stop_bits = stop_bits;
        line_coding_is_change = true;
    }

    if (usb_bridge.last_line_coding.parity != parity)
    {
        usb_bridge.last_line_coding.parity = parity;
        line_coding_is_change = true;
    }

    if (usb_bridge.last_line_coding.data_bits != data_bits)
    {
        usb_bridge.last_line_coding.data_bits = data_bits;
        line_coding_is_change = true;
    }

    if (line_coding_is_change)
    {
        xQueueSend(usb_bridge.line_coding_queue, &usb_bridge.last_line_coding, pdTICKS_TO_MS(10));
        ESP_LOGI(TAG, "host require bit_rate=%" PRIu32 " stop_bits=%u parity=%u data_bits=%u", bit_rate, stop_bits, parity, data_bits);
    }
}

usb_bridge_error_t usb_bridge_line_coding_change_recv(usb_bridge_line_coding_t *usb_bridge_line_coding)
{
    if (usb_bridge_line_coding == NULL)
        return USB_BRIDGE_LINE_CODING_CHANGE_RECV_ERROR;

    if (xQueueReceive(usb_bridge.line_coding_queue, usb_bridge_line_coding, pdTICKS_TO_MS(10)) != pdPASS)
    {
        ESP_LOGD(TAG, "Line coding queue receive error.");
        return USB_BRIDGE_LINE_CODING_CHANGE_RECV_ERROR;
    }

    return USB_BRIDGE_OK;
}

static void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;

    bool line_state_is_change = false;

    if (usb_bridge.last_line_state_changed_data.dtr != dtr)
    {
        usb_bridge.last_line_state_changed_data.dtr = dtr;
        line_state_is_change = true;
    }

    if (usb_bridge.last_line_state_changed_data.rts != rts)
    {
        usb_bridge.last_line_state_changed_data.rts = rts;
        line_state_is_change = true;
    }

    if (line_state_is_change)
    {
        xQueueSend(usb_bridge.line_state_changed_data_queue, &usb_bridge.last_line_state_changed_data, pdTICKS_TO_MS(10));
        ESP_LOGI(TAG, "Line state changed! dtr:%d, rts:%d ", dtr, rts);
    }
}

usb_bridge_error_t usb_bridge_line_state_change_recv(usb_bridge_line_state_changed_data_t *usb_bridge_line_state_changed_data)
{
    if (usb_bridge_line_state_changed_data == NULL)
        return USB_BRIDGE_LINE_STATE_CHANGE_RECV_ERROR;

    if (xQueueReceive(usb_bridge.line_state_changed_data_queue, usb_bridge_line_state_changed_data, pdTICKS_TO_MS(10)) != pdPASS)
    {
        ESP_LOGD(TAG, "Line state change queue receive error.");
        return USB_BRIDGE_LINE_STATE_CHANGE_RECV_ERROR;
    }

    return USB_BRIDGE_OK;
}

void usb_bridge_tx_task(void *argument)
{
    size_t rx_data_size = 0;

    while (1)
    {
        usb_bridge_error_t error = usb_bridge_read_ringbuffer(usb_bridge.usb_tx_ringbuffer, usb_bridge_write_buffer, USB_BRIDGE_READ_BUFFER_SIZE, &rx_data_size);
        if (error == USB_BRIDGE_OK)
        {
            tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, usb_bridge_write_buffer, rx_data_size);
            uint32_t timeout_ms = 10 * (rx_data_size / 64 / 19 + 1);
            tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, timeout_ms);
            // ESP_LOGI(TAG, "TX");
        }
    }
}

size_t usb_bridge_tx(uint8_t *tx_buffer, size_t tx_buffer_size)
{
    if (xRingbufferSend(usb_bridge.usb_tx_ringbuffer, (const void *)tx_buffer, tx_buffer_size, pdTICKS_TO_MS(10)) != pdTRUE)
        ESP_LOGW(TAG, "Cdcacm xringbuffer send error.");

    return tx_buffer_size;
}

static void tinyusb_cdcacm_rx_forward_callback(int itf, cdcacm_event_t *event)
{
    size_t rx_size = 0;

    esp_err_t ret = tinyusb_cdcacm_read(itf, usb_bridge_read_buffer, USB_BRIDGE_READ_BUFFER_SIZE, &rx_size);
    if (ret == ESP_OK)
    {
        // ESP_LOG_BUFFER_HEX(TAG, rx_buffer, rx_size);
        if (xRingbufferSend(usb_bridge.usb_rx_forward_ringbuffer, (const void *)usb_bridge_read_buffer, rx_size, 0) != pdTRUE)
            ESP_LOGW(TAG, "Rx ring buffer send error.");
    }
    else
    {
        ESP_LOGE(TAG, "usb read error");
    }
}

usb_bridge_error_t usb_bridge_rx(uint8_t *rx_buffer, size_t rx_buffer_size, size_t *rx_data_size)
{
    if (rx_buffer == NULL || rx_data_size == NULL)
        return USB_BRIDGE_RX_ERROR;

    return usb_bridge_read_ringbuffer(usb_bridge.usb_rx_forward_ringbuffer, rx_buffer, rx_buffer_size, rx_data_size);
}

void usb_bridge_rx_forward_task(void *arguments)
{
    size_t rx_data_size = 0;

    while (1)
    {
        usb_bridge_error_t error = usb_bridge_read_ringbuffer(usb_bridge.usb_rx_forward_ringbuffer, usb_bridge_read_buffer, USB_BRIDGE_READ_BUFFER_SIZE, &rx_data_size);
        if (error == USB_BRIDGE_OK)
        {
            if (usb_bridge.forward_callback != NULL)
            {
                usb_bridge.forward_callback(usb_bridge_read_buffer, rx_data_size);
            }
            // ESP_LOG_BUFFER_HEX(TAG, usb_bridge_uart_rx_buffer, rx_data_size);
        }
    }
}

void usb_bridge_line_coding_changed_task(void *arguments)
{
    usb_bridge_line_coding_t line_coding;

    while (1)
    {
        if (usb_bridge_line_coding_change_recv(&line_coding) == USB_BRIDGE_OK)
        {
            if (usb_bridge.line_coding_changed_callback != NULL)
                usb_bridge.line_coding_changed_callback(line_coding.bit_rate, line_coding.stop_bits, line_coding.parity, line_coding.data_bits);
        }
    }
}

void usb_bridge_line_state_changed_task(void *arguments)
{
    usb_bridge_line_state_changed_data_t line_state_changed_data;

    while (1)
    {
        if (usb_bridge_line_state_change_recv(&line_state_changed_data) == USB_BRIDGE_OK)
        {
            if (usb_bridge.line_state_changed_callback != NULL)
                usb_bridge.line_state_changed_callback(line_state_changed_data.dtr, line_state_changed_data.rts);
        }
    }
}

usb_bridge_error_t usb_bridge_init(usb_bridge_config_t *usb_bridge_config)
{
    if (usb_bridge_config == NULL)
        return USB_BRIDGE_INIT_ERROR;

    usb_bridge.tx_data_task = NULL;
    usb_bridge.rx_data_task = NULL;
    usb_bridge.line_coding_changed_task = NULL;
    usb_bridge.line_state_changed_task = NULL;

    usb_bridge.usb_tx_ringbuffer = xRingbufferCreate(USB_BRIDGE_WRITE_BUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
    usb_bridge.usb_rx_forward_ringbuffer = xRingbufferCreate(USB_BRIDGE_READ_BUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);

    usb_bridge.line_coding_queue = xQueueCreate(10, sizeof(usb_bridge_line_coding_t));
    usb_bridge.line_state_changed_data_queue = xQueueCreate(10, sizeof(usb_bridge_line_state_changed_data_t));

    usb_bridge.forward_callback = usb_bridge_config->forward_callback;
    usb_bridge.line_coding_changed_callback = usb_bridge_config->line_coding_changed_callback;
    usb_bridge.line_state_changed_callback = usb_bridge_config->line_state_changed_callback;

    static const uint16_t cdc_desc_config_len = TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN;
    static const uint8_t cdc_desc_configuration[] = {
        TUD_CONFIG_DESCRIPTOR(1, 2, 0, cdc_desc_config_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),
        TUD_CDC_DESCRIPTOR(0, 4, 0x81, 8, 0x02, 0x82, (TUD_OPT_HIGH_SPEED ? 512 : 64)),
    };

    tinyusb_config_t tinyusb_config = TINYUSB_DEFAULT_CONFIG();
    tinyusb_config.descriptor.device = &cdc_device_descriptor;
    tinyusb_config.descriptor.full_speed_config = cdc_desc_configuration;

    ESP_ERROR_CHECK(tinyusb_driver_install(&tinyusb_config));

    tinyusb_config_cdcacm_t acm_cfg = {
        .cdc_port = TINYUSB_CDC_ACM_0,
        .callback_rx = &tinyusb_cdcacm_rx_forward_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = &tinyusb_cdc_line_coding_changed_callback};

    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_cfg));

    ESP_LOGI(TAG, "USB initialization DONE");

    return USB_BRIDGE_OK;
}

usb_bridge_error_t usb_bridge_task_start()
{
    xTaskCreate(usb_bridge_tx_task, "usb_bridge_tx_task", 1024 * 4, &usb_bridge, 8, &usb_bridge.tx_data_task);
    xTaskCreate(usb_bridge_rx_forward_task, "usb_bridge_rx_forward_task", 1024 * 4, &usb_bridge, 8, &usb_bridge.rx_data_task);
    xTaskCreate(usb_bridge_line_coding_changed_task, "usb_bridge_line_coding_changed_task", 1024 * 4, &usb_bridge, 8, &usb_bridge.line_coding_changed_task);
    xTaskCreate(usb_bridge_line_state_changed_task, "usb_bridge_line_state_changed_task", 1024 * 4, &usb_bridge, 8, &usb_bridge.line_state_changed_task);

    return USB_BRIDGE_OK;
}

usb_bridge_error_t usb_bridge_deinit()
{
    return USB_BRIDGE_OK;
}
