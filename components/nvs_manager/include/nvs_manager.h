#ifndef nvs_manager_h
#define nvs_manager_h

#include "stdint.h"
#include "stdbool.h"

typedef enum
{
    NVS_MANAGER_OK = 0,
    NVS_MANAGER_ERROR_INIT,
    NVS_MANAGER_ERROR_DEINIT,
    NVS_MANAGER_ERROR_NOT_INIT,
    NVS_MANAGER_ERROR_SET,
    NVS_MANAGER_ERROR_GET,
    NVS_MANAGER_ERROR_NOT_FOUND,
    NVS_MANAGER_ERROR_ERASE,
    NVS_MANAGER_ERROR_ERASE_ALL,
    NVS_MANAGER_ERROR_EXISTS,
    NVS_MANAGER_ERROR_INVALID_ARG,
} nvs_manager_error_t;

nvs_manager_error_t nvs_manager_init(char *namespace_name);
nvs_manager_error_t nvs_manager_deinit(void);

nvs_manager_error_t nvs_manager_set_blob(char *key, void *value, size_t length);

nvs_manager_error_t nvs_manager_get_blob(char *key, void *out_value, size_t *length);

nvs_manager_error_t nvs_manager_erase(char *key);
nvs_manager_error_t nvs_manager_erase_all(void);

nvs_manager_error_t nvs_manager_exists(char *key, bool *out_exists);

#endif
