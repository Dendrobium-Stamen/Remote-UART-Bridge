#ifndef web_h
#define web_h

typedef struct
{
} web_config_t;

typedef enum
{
    WEB_OK,
    WEB_ERROR_INIT,
    WEB_ERROR_DEINIT,
} web_error_t;

web_error_t web_init(web_config_t *config);
web_error_t web_deinit(void);

#endif
