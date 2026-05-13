#include "stdio.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "nvs_manager.h"

static const char *TAG = "Nvs Manager";

static char nvs_manager_namespace[NVS_NS_NAME_MAX_SIZE];

nvs_manager_error_t nvs_manager_init(char *namespace_name)
{
    if (namespace_name == NULL)
    {
        return NVS_MANAGER_ERROR_INVALID_ARG;
    }

    if (strlen(namespace_name) > NVS_NS_NAME_MAX_SIZE - 1)
    {
        ESP_LOGE(TAG, "Namespace name too long");
        return NVS_MANAGER_ERROR_INIT;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    memset(nvs_manager_namespace, 0, sizeof(nvs_manager_namespace));
    strncpy(nvs_manager_namespace, namespace_name, strlen(namespace_name));

    ESP_LOGI(TAG, "Initialized with namespace '%s'", nvs_manager_namespace);
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_set_blob(char *key, void *value, size_t length)
{
    if (key == NULL || value == NULL)
        return NVS_MANAGER_ERROR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(nvs_manager_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open write failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_SET;
    }

    err = nvs_set_blob(handle, key, value, length);
    if (err != ESP_OK)
    {
        nvs_close(handle);
        ESP_LOGE(TAG, "nvs_set_blob failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_SET;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_SET;
    }
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_get_blob(char *key, void *out_value, size_t *length)
{
    if (key == NULL || length == NULL)
        return NVS_MANAGER_ERROR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(nvs_manager_namespace, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open read failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_GET;
    }

    err = nvs_get_blob(handle, key, out_value, length);
    nvs_close(handle);
    if (err == ESP_ERR_NVS_NOT_FOUND)
        return NVS_MANAGER_ERROR_NOT_FOUND;
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_get_blob failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_GET;
    }
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_erase(char *key)
{
    if (key == NULL)
        return NVS_MANAGER_ERROR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(nvs_manager_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open write failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE;
    }

    err = nvs_erase_key(handle, key);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);
        return NVS_MANAGER_ERROR_NOT_FOUND;
    }
    if (err != ESP_OK)
    {
        nvs_close(handle);
        ESP_LOGE(TAG, "nvs_erase_key failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE;
    }
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_erase_all(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(nvs_manager_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open write failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE_ALL;
    }

    err = nvs_erase_all(handle);
    if (err != ESP_OK)
    {
        nvs_close(handle);
        ESP_LOGE(TAG, "nvs_erase_all failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE_ALL;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_ERASE_ALL;
    }
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_exists(char *key, bool *out_exists)
{
    if (key == NULL || out_exists == NULL)
        return NVS_MANAGER_ERROR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(nvs_manager_namespace, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open read failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_EXISTS;
    }

    err = nvs_find_key(handle, key, NULL);
    nvs_close(handle);
    if (err == ESP_OK)
    {
        *out_exists = true;
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        *out_exists = false;
    }
    else
    {
        ESP_LOGE(TAG, "nvs_find_key failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_EXISTS;
    }
    return NVS_MANAGER_OK;
}

nvs_manager_error_t nvs_manager_deinit(void)
{
    esp_err_t err = nvs_flash_deinit();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS deinit failed: %s", esp_err_to_name(err));
        return NVS_MANAGER_ERROR_DEINIT;
    }

    ESP_LOGI(TAG, "Deinitialized");
    return NVS_MANAGER_OK;
}
