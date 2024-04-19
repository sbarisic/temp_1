#include <core2.h>

#include <esp_http_server.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>
#include <esp_wifi.h>

// https://github.com/espressif/esp-idf/blob/v5.2.1/examples/protocols/http_server/simple/main/main.c#L368
httpd_handle_t http_server = NULL;

esp_err_t root_get_handler(httpd_req_t *req)
{
    dprintf("Serving / response!\n");

    size_t index_len = 0;
    void *index_buf = core2_file_read_all("/sd/http/index.html", &index_len);

    if (index_buf != NULL)
    {
        httpd_resp_send(req, (const char*)index_buf, index_len);
        core2_free(index_buf);
    }

    return ESP_OK;
}

const httpd_uri_t root_handler = {.uri = "/", .method = HTTP_GET, .handler = root_get_handler};

bool core2_http_start()
{
    dprintf("core2_http_start()\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    dprintf("core2_http_start() - Starting HTTP server on port %d\n", config.server_port);

    if (httpd_start(&http_server, &config) == ESP_OK)
    {
        dprintf("core2_http_start() - Registering URI handlers\n");
        httpd_register_uri_handler(http_server, &root_handler);
    }
    else
    {
        dprintf("core2_http_start() - httpd_start() FAILED\n");
    }

    return true;
}
