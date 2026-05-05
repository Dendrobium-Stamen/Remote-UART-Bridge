#ifndef wifi_manager_h
#define wifi_manager_h

typedef enum
{
    WIFI_MANAGER_OK = 0,
    WIFI_MANAGER_ERROR,
} wifi_manager_error_t;

wifi_manager_error_t wifi_manager_init(void);

#endif
