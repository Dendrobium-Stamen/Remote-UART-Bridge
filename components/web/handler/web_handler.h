#ifndef web_handler_h
#define web_handler_h

#include "esp_http_server.h"

esp_err_t page_handler(httpd_req_t *req);
esp_err_t api_status_handler(httpd_req_t *req);
esp_err_t api_peers_handler(httpd_req_t *req);
esp_err_t api_scan_handler(httpd_req_t *req);

#endif
