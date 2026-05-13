#include "stdio.h"

// #include "esp_log.h"
#include "esp_http_server.h"

/* ---- 嵌入 HTML ---- */
extern const uint8_t espnow_web_html_start[] asm("_binary_espnow_html_html_start");
extern const uint8_t espnow_web_html_end[] asm("_binary_espnow_html_html_end");

/* ---- GET / : 返回页面 ---- */
esp_err_t espnow_page_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, (const char *)espnow_web_html_start, espnow_web_html_end - espnow_web_html_start);
}
