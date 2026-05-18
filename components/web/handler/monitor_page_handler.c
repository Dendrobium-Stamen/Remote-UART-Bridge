#include "stdio.h"

// #include "esp_log.h"
#include "esp_http_server.h"

extern const uint8_t monitor_web_html_start[] asm("_binary_monitor_html_html_start");
extern const uint8_t monitor_web_html_end[] asm("_binary_monitor_html_html_end");

esp_err_t monitor_page_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, (const char *)monitor_web_html_start, monitor_web_html_end - monitor_web_html_start);
}
