#include "stdio.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

static const char *TAG = "Memory Display";

void print_ram_bar(void)
{
    size_t total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t free = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t used = total - free;
    int pct = (total > 0) ? (int)((float)used / total * 100.0f) : 0;

    int bar_len = 30;
    int filled = bar_len * pct / 100;

    char bar[32];
    for (int i = 0; i < bar_len; i++)
    {
        bar[i] = (i < filled) ? '#' : '-';
    }
    bar[bar_len] = '\0';

    ESP_LOGI(TAG, "RAM [%s] %d%% used  (%u / %u bytes)",
             bar, pct, used, total);
}