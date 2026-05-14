#include "stdio.h"
#include "math.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "CPU";

static volatile uint64_t total_idle_time_us = 0;
static volatile uint64_t last_idle_time_us = 0;

// 空闲钩子
bool IRAM_ATTR idle_hook(void)
{
    static uint64_t last_wake = 0;
    uint64_t now = esp_timer_get_time();

    if (last_wake != 0)
    {
        total_idle_time_us += (now - last_wake);
    }
    last_wake = now;
    return false;
}

void cpu_monitor_task(void *arg)
{
    esp_register_freertos_idle_hook_for_cpu(idle_hook, 0);

    while (1)
    {
        uint64_t start_idle = total_idle_time_us;
        uint64_t start_time = esp_timer_get_time();

        vTaskDelay(pdMS_TO_TICKS(1000));

        uint64_t end_idle = total_idle_time_us;
        uint64_t end_time = esp_timer_get_time();

        uint64_t elapsed = end_time - start_time;
        uint64_t idle = end_idle - start_idle;

        float usage = (1.0f - ((float)idle / (float)elapsed)) * 100.0f;
        usage = fmaxf(0.0f, fminf(100.0f, usage));

        ESP_LOGI(TAG, "CPU Usage: %.2f%% | Total: %llu ms | Idle: %llu ms",
                 usage, elapsed / 1000, idle / 1000);
    }
}
