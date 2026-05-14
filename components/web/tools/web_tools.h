#ifndef web_tools_h
#define web_tools_h

#include "stdint.h"

#include "esp_http_server.h"

#include "cJSON.h"

#define WEB_TOOLS_MAC_TO_STR_LENGTH 18

void web_tools_mac_to_str(const uint8_t *mac, char *out);
bool web_tools_str_to_mac(const char *str, uint8_t *out);

esp_err_t web_tools_send_json_ok(httpd_req_t *req);
esp_err_t web_tools_send_json_error(httpd_req_t *req, const char *msg);
esp_err_t web_tools_send_json_resp(httpd_req_t *req, cJSON *root);

int web_tools_read_body(httpd_req_t *req, char *buffer, size_t buffer_size);

#endif
