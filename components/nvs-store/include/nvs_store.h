#ifndef nvs_store_h
#define nvs_store_h

#include "stdint.h"

typedef enum
{
    NVS_STORE_OK,
    NVS_STORE_ERROR_SAVE,
    NVS_STORE_ERROR_LOAD,
    NVS_STORE_ERROR_ADD,
    NVS_STORE_ERROR_COMPARE,
    NVS_STORE_ERROR_DELETE,
} nvs_store_error_t;

#define NVS_STORE_MAX_PEERS 10

typedef struct
{
    uint8_t count;
    uint8_t macs[NVS_STORE_MAX_PEERS][6];
} nvs_store_t;

nvs_store_error_t nvs_store_save(nvs_store_t *nvs_store);
nvs_store_error_t nvs_store_load(nvs_store_t *nvs_store);
nvs_store_error_t nvs_store_compare(nvs_store_t *nvs_store, uint8_t *peer_mac);
nvs_store_error_t nvs_store_add(nvs_store_t *nvs_store, uint8_t *peer_mac);
nvs_store_error_t nvs_store_delete(nvs_store_t *nvs_store, uint8_t *peer_mac);

#endif
