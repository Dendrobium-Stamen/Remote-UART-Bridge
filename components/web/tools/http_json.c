#include "stdio.h"

#include "esp_http_server.h"

#include "cJSON.h"

esp_err_t web_tools_send_json_resp(httpd_req_t *req, cJSON *root)
{
    char *json = cJSON_PrintUnformatted(root);
    if (!json)
        return ESP_FAIL;
    httpd_resp_set_type(req, "application/json");
    esp_err_t ret = httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    cJSON_free(json);
    return ret;
}

esp_err_t web_tools_send_json_ok(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "ok", true);
    esp_err_t ret = web_tools_send_json_resp(req, root);
    cJSON_Delete(root);
    return ret;
}

esp_err_t web_tools_send_json_error(httpd_req_t *req, const char *msg)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "ok", false);
    cJSON_AddStringToObject(root, "error", msg ? msg : "unknown error");
    esp_err_t ret = web_tools_send_json_resp(req, root);
    cJSON_Delete(root);
    return ret;
}
