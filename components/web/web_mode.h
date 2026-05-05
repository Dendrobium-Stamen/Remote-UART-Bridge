#ifndef web_mode_h
#define web_mode_h

#include "esp_http_server.h"

typedef struct
{
    httpd_handle_t server;
} web_t;

#endif
