#include "stdio.h"

#include "esp_http_server.h"

int web_tools_read_body(httpd_req_t *req, char *buffer, size_t buffer_size)
{
    int total = 0;
    int remaining = req->content_len;
    while (remaining > 0 && total < (int)buffer_size - 1)
    {
        int to_read = remaining > ((int)buffer_size - 1 - total)
                          ? ((int)buffer_size - 1 - total)
                          : remaining;
        int received = httpd_req_recv(req, buffer + total, to_read);
        if (received <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
                continue;
            return -1;
        }
        total += received;
        remaining -= received;
    }
    buffer[total] = '\0';
    return total;
}
