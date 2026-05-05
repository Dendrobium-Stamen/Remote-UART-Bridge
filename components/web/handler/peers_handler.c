#include "stdio.h"

#include "esp_log.h"
#include "esp_http_server.h"

/* ---- GET /api/peers ---- */
static esp_err_t api_peers_handler(httpd_req_t *req)
{
    // cJSON *arr = get_peers_json();
    // const char *resp = cJSON_PrintUnformatted(arr);
    // httpd_resp_set_type(req, "application/json");
    // httpd_resp_sendstr(req, resp);
    // cJSON_free((void *)resp);
    // cJSON_Delete(arr);
    return ESP_OK;
}
