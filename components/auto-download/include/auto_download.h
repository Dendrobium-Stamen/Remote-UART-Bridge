#ifndef auto_download_h
#define auto_download_h

#include "stdbool.h"

typedef struct
{
    int dtr_gpio_num;
    int rts_gpio_num;
} auto_download_config_t;

typedef enum
{
    AUTO_DOWNLOAD_OK = 0,
    AUTO_DOWNLOAD_INIT_ERROR,
    AUTO_DOWNLOAD_DEINIT_ERROR,
    AUTO_DOWNLOAD_GPIO_ERROR,
} auto_download_error_t;

auto_download_error_t auto_download_init(auto_download_config_t *config);
bool auto_download_set_gpio_level(bool dtr_level, bool rts_level);
auto_download_error_t auto_download_deinit();

#endif
