
#include <core2.h>

#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>
#include <esp_http_server.h>

httpd_handle_t http_server = NULL;

// https://github.com/espressif/esp-idf/blob/v5.2.1/examples/protocols/http_server/simple/main/main.c#L368

/* An HTTP GET handler */
esp_err_t hello_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = (char *)malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            dprintf("Found header => Host: %s\n", buf);
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    httpd_resp_send(req, "Hello HTTP World!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

const httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = hello_get_handler};

void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        dprintf("Starting webserver\n");
        *server = http_server;
    }
}

bool core2_http_start()
{
    dprintf("core2_http_start()\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    dprintf("Starting server on port: '%d'\n", config.server_port);

    if (httpd_start(&http_server, &config) == ESP_OK)
    {
        // Set URI handlers
        dprintf("Registering URI handlers\n");
        httpd_register_uri_handler(http_server, &hello);
    }

    dprintf("Registering IP_EVENT_STA_GOT_IP handler\n");
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, NULL);

    return true;
}
