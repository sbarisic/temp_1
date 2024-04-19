#include <core2.h>

#include <esp_http_server.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>
#include <esp_wifi.h>

esp_err_t root_get_handler(httpd_req_t *req);

httpd_handle_t http_server = NULL;

const httpd_uri_t root_handler = {
    .uri = "*",
    .method = HTTP_GET,
    .handler = root_get_handler};

esp_err_t root_get_handler(httpd_req_t *req)
{
    dprintf("Serving URI: %s\n", req->uri);

    if (req->uri == NULL || (strlen(req->uri) > 100))
    {
        dprintf("FAIL, Request URI '%s'\n", req->uri);
        return ESP_FAIL;
    }

    // Trim starting /
    char uri_normalized[128] = {0};
    strcpy(uri_normalized, req->uri + 1);

    // Convert to filename
    char buffer[128] = {0};
    sprintf(buffer, "/sd/http/%s", uri_normalized);

    size_t index_len = 0;
    void *index_buf = core2_file_read_all(buffer, &index_len);

    if (index_buf != NULL)
    {
        if (core2_string_ends_with(req->uri, ".css"))
            httpd_resp_set_type(req, "text/css");
        else if (core2_string_ends_with(req->uri, ".js"))
            httpd_resp_set_type(req, "text/javascript");
        else if (core2_string_ends_with(req->uri, ".html"))
            httpd_resp_set_type(req, "text/html");
        else if (core2_string_ends_with(req->uri, ".png"))
            httpd_resp_set_type(req, "image/png");
        else
            httpd_resp_set_type(req, "text/plain");

        httpd_resp_send(req, (const char *)index_buf, index_len);
        core2_free(index_buf);
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_OK;
}

bool core2_http_start()
{
    dprintf("core2_http_start()\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the httpd server
    dprintf("core2_http_start() - Starting HTTP server on port %d\n", config.server_port);

    if (httpd_start(&http_server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(http_server, &root_handler);
    }
    else
    {
        dprintf("core2_http_start() - httpd_start() FAILED\n");
    }

    return true;
}
