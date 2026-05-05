#include "stdio.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "auto_download.h"

static const char *TAG = "auto_download";

typedef struct
{
    gpio_num_t dtr_gpio_num;
    gpio_num_t rts_gpio_num;
} auto_download_t;

auto_download_t auto_download = {
    .dtr_gpio_num = GPIO_NUM_NC,
    .rts_gpio_num = GPIO_NUM_NC,
};

auto_download_error_t auto_download_init(auto_download_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Invalid config");
        return AUTO_DOWNLOAD_INIT_ERROR;
    }

    auto_download.dtr_gpio_num = config->dtr_gpio_num;
    auto_download.rts_gpio_num = config->rts_gpio_num;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << auto_download.dtr_gpio_num) | (1ULL << auto_download.rts_gpio_num),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };

    gpio_config(&io_conf);

    gpio_set_level(auto_download.dtr_gpio_num, 1);
    gpio_set_level(auto_download.rts_gpio_num, 1);

    return AUTO_DOWNLOAD_OK;
}

auto_download_error_t auto_download_set_gpio_level(bool dtr_level, bool rts_level)
{
    gpio_set_level(auto_download.dtr_gpio_num, dtr_level);
    gpio_set_level(auto_download.rts_gpio_num, rts_level);

    return AUTO_DOWNLOAD_OK;
}

auto_download_error_t auto_download_deinit()
{
    return AUTO_DOWNLOAD_OK;
}
