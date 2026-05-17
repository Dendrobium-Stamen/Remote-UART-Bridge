#ifndef web_handler_h
#define web_handler_h

#include "esp_http_server.h"

esp_err_t espnow_page_handler(httpd_req_t *req);
esp_err_t monitor_page_handler(httpd_req_t *req);

esp_err_t system_status_handler(httpd_req_t *req);
esp_err_t system_label_handler(httpd_req_t *req);

esp_err_t espnow_peers_handler(httpd_req_t *req);
esp_err_t espnow_scan_handler(httpd_req_t *req);
esp_err_t espnow_peer_add_handler(httpd_req_t *req);
esp_err_t espnow_peer_delete_handler(httpd_req_t *req);
esp_err_t espnow_peer_enable_handler(httpd_req_t *req);
esp_err_t espnow_peer_disable_handler(httpd_req_t *req);

// esp_err_t api_status_handler(httpd_req_t *req);
// esp_err_t api_peers_handler(httpd_req_t *req);
// esp_err_t api_scan_handler(httpd_req_t *req);
// esp_err_t api_peer_add_handler(httpd_req_t *req);
// esp_err_t api_peer_del_handler(httpd_req_t *req);

#endif
