#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#include "auto_download.h"

static const char *TAG = "auto_download";

static esp_timer_handle_t auto_download_timer = NULL;

typedef struct
{
    gpio_num_t dtr_gpio_num;
    gpio_num_t rts_gpio_num;
} auto_download_t;

auto_download_t auto_download = {
    .dtr_gpio_num = GPIO_NUM_NC,
    .rts_gpio_num = GPIO_NUM_NC,
};

static void auto_download_timer_cb(void *arg)
{
    gpio_set_level(auto_download.dtr_gpio_num, 1);
    gpio_set_level(auto_download.rts_gpio_num, 1);
}

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

bool auto_download_set_gpio_level(bool dtr_level, bool rts_level)
{
    static bool download_prepared = false;

    if (auto_download_timer)
        esp_timer_stop(auto_download_timer);

    if (dtr_level == 0 && rts_level == 1)
    {
        gpio_set_level(auto_download.dtr_gpio_num, 0);
        gpio_set_level(auto_download.rts_gpio_num, 1);
        download_prepared = true;
    }
    else if (dtr_level == 1 && rts_level == 0)
    {
        if (download_prepared)
        {
            gpio_set_level(auto_download.dtr_gpio_num, 1);
            gpio_set_level(auto_download.rts_gpio_num, 0);
        }
        else
        {
            gpio_set_level(auto_download.dtr_gpio_num, 1);
            gpio_set_level(auto_download.rts_gpio_num, 0);
        }
    }
    else if (dtr_level == 1 && rts_level == 1)
    {
        if (auto_download_timer == NULL)
        {
            const esp_timer_create_args_t timer_args = {
                .callback = auto_download_timer_cb,
                .name = "auto_dl_timer",
            };
            ESP_ERROR_CHECK(esp_timer_create(&timer_args, &auto_download_timer));
        }
        esp_timer_start_once(auto_download_timer, 10 * 1000);
    }
    else
    {
        gpio_set_level(auto_download.dtr_gpio_num, 1);
        gpio_set_level(auto_download.rts_gpio_num, 1);
        download_prepared = false;
    }

    return true;
}

auto_download_error_t auto_download_deinit()
{
    return AUTO_DOWNLOAD_OK;
}
