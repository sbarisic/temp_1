#include <core2.h>

#include <esp_http_server.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>
#include <esp_wifi.h>

//==========================================================

httpd_handle_t http_server = NULL;

esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t shell_post_handler(httpd_req_t *req);

const httpd_uri_t root_handler = {.uri = "*", .method = HTTP_GET, .handler = root_get_handler};
const httpd_uri_t shell_handler = {.uri = "/shell", .method = HTTP_POST, .handler = shell_post_handler};

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

esp_err_t shell_post_handler(httpd_req_t *req)
{
    size_t len = req->content_len;
    dprintf("/shell - Content length: %d\n", len);

    if (len > 2048 || len <= 0)
    {
        httpd_resp_send_500(req);
        return ESP_OK;
    }

    char *buffer = (char *)core2_malloc(len + 1);
    if (buffer == NULL)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "core2_malloc() returned NULL");
        return ESP_OK;
    }

    int recv_res = httpd_req_recv(req, buffer, len);
    dprintf("httpd_req_recv() - %d\n", recv_res);

    if (recv_res <= 0)
    {
        core2_free(buffer);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "httpd_req_recv() errored");
        return ESP_OK;
    }

    // Null terminate the buffer
    buffer[len] = 0;

    char *print_buffer = (char *)core2_malloc(4096);
    size_t printbuf_idx = 0;

    // Invoke command here
    core2_shell_func_params_t params;
    params.ud1 = req;
    params.ud2 = print_buffer;
    params.ud3 = &printbuf_idx;

    params.print = [](void *self, const char *str)
    {
        core2_shell_func_params_t *self_params = (core2_shell_func_params_t *)self;
        size_t *printbuf_idx_p = (size_t *)self_params->ud3;
        size_t len = strlen(str);

        memcpy((void *)((size_t)self_params->ud2 + (*printbuf_idx_p)), str, len);

        *printbuf_idx_p = (*printbuf_idx_p) + len;
    };

    if (!core2_shell_invoke(buffer, &params))
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Shell command not found");
    }
    else
    {
        httpd_resp_send(req, print_buffer, HTTPD_RESP_USE_STRLEN);
    }

    core2_free(print_buffer);
    core2_free(buffer);
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
        httpd_register_uri_handler(http_server, &shell_handler);
    }
    else
    {
        dprintf("core2_http_start() - httpd_start() FAILED\n");
    }

    return true;
}
