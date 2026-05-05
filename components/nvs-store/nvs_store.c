#include "stdio.h"
#include "string.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "nvs_store.h"

static const char *TAG = "NVS store";

#define NVS_NAMESPACE "nvs_store"
#define NVS_KEY_PEERS "peers"

nvs_store_error_t nvs_store_save(nvs_store_t *nvs_store)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK)
        return NVS_STORE_ERROR_SAVE;

    err = nvs_set_blob(h, NVS_KEY_PEERS, nvs_store, sizeof(nvs_store_t));
    if (err == ESP_OK)
    {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return NVS_STORE_OK;
}

nvs_store_error_t nvs_store_load(nvs_store_t *nvs_store)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
        return NVS_STORE_ERROR_LOAD;

    size_t length = sizeof(nvs_store_t);
    err = nvs_get_blob(nvs_handle, NVS_KEY_PEERS, nvs_store, &length);
    nvs_close(nvs_handle);
    return NVS_STORE_OK;
}

nvs_store_error_t nvs_store_compare(nvs_store_t *nvs_store, uint8_t *peer_mac)
{
    for (int i = 0; i < nvs_store->count; i++)
        if (memcmp(nvs_store->macs[i], peer_mac, 6) == 0)
            return NVS_STORE_ERROR_COMPARE;

    return NVS_STORE_OK;
}

nvs_store_error_t nvs_store_add(nvs_store_t *nvs_store, uint8_t *peer_mac)
{
    if (nvs_store == NULL || peer_mac == NULL)
        return NVS_STORE_ERROR_ADD;

    if (nvs_store->count >= NVS_STORE_MAX_PEERS)
        return NVS_STORE_ERROR_ADD;

    memcpy(nvs_store->macs[nvs_store->count], peer_mac, 6);
    nvs_store->count++;

    return nvs_store_save(nvs_store);
}

nvs_store_error_t nvs_store_delete(nvs_store_t *nvs_store, uint8_t *peer_mac)
{
    if (nvs_store == NULL || peer_mac == NULL)
        return NVS_STORE_ERROR_ADD;

    if (nvs_store->count >= NVS_STORE_MAX_PEERS)
        return NVS_STORE_ERROR_ADD;

    for (int i = 0; i < nvs_store->count; i++)
    {
        if (memcmp(nvs_store->macs[i], peer_mac, 6) == 0)
        {
            memset(nvs_store->macs[i], 0, 6);
            nvs_store->count--;
            return nvs_store_save(nvs_store);
        }
    }

    return NVS_STORE_ERROR_DELETE;
}
