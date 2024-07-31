#include <core2.h>
#include <core2_update.h>

#include <esp_http_server.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>
#include <esp_wifi.h>

//==========================================================

httpd_handle_t http_server = NULL;

esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t firmware_post_handler2(httpd_req_t *req);
esp_err_t firmware_post_handler(httpd_req_t *req);
esp_err_t shell_post_handler(httpd_req_t *req);

const httpd_uri_t root_handler = {.uri = "*", .method = HTTP_GET, .handler = root_get_handler};
const httpd_uri_t shell_handler = {.uri = "/shell", .method = HTTP_POST, .handler = shell_post_handler};
const httpd_uri_t firmware_handler = {.uri = "/firmware", .method = HTTP_POST, .handler = firmware_post_handler2};

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
    // void *index_buf = core2_file_read_all(buffer, &index_len);

    FILE *f = NULL;
    size_t file_len = 0;

    if (core2_file_exists(buffer))
    {
        f = core2_file_open(buffer, "r");
        file_len = core2_file_length(f);

        size_t chunk_size = 1024;
        char *temp_buf = (char *)core2_malloc(chunk_size);

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

        // httpd_resp_send(req, (const char *)index_buf, index_len);
        // core2_free(index_buf);

        // TODO: error checking
        while (true)
        {
            size_t read_len = fread(temp_buf, 1, chunk_size, f);
            httpd_resp_send_chunk(req, temp_buf, read_len);

            if (read_len != chunk_size)
            {
                break;
            }
        }

        httpd_resp_send_chunk(req, NULL, 0);
        core2_free(temp_buf);
        core2_file_close(f);
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_OK;
}

void restart_task(void *a)
{
    core2_sleep(1000);
    esp_restart();
    vTaskDelete(NULL);
}

httpd_req_t *request;

esp_err_t write_from_http(ota_write_func ota_write)
{
    dprintf("Receiving firmware (%ld bytes)\n", request->content_len);

    /* Temporary buffer */
    const size_t buf_size = 4096;
    char *buf = (char *)core2_malloc(buf_size);
    int received;

    size_t len = request->content_len;
    size_t remaining = request->content_len; // Content length of the request gives the size of the file being uploaded
    int last_print_percent = -1;

    while (remaining > 0)
    {
        if ((received = httpd_req_recv(request, buf, min(remaining, buf_size))) <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
                continue;

            core2_free(buf);
            dprintf("firmware reception failed!\n");
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive firmware");
            return ESP_FAIL;
        }

        // Write buffer content to file on storage
        if (received)
        {
            int percent = ((len - remaining) * 100) / len;
            if (last_print_percent != percent)
            {
                last_print_percent = percent;
                dprintf("ota_write (%d %%)\n", percent);
            }

            esp_err_t err = ota_write(buf, received);

            if (err != ESP_OK)
            {
                core2_free(buf);
                return err;
            }
        }

        remaining -= received;
    }

    dprintf("ota_write (%d %%)\n", 100);
    core2_free(buf);
    return ESP_OK;
}

#define MAX_FILE_SIZE_STR "2 mb"
esp_err_t firmware_post_handler2(httpd_req_t *req)
{
    const int MAX_FILE_SIZE = 2000000;
    const char *filepath = "/sd/firmware.bin";

    if (req->content_len > MAX_FILE_SIZE)
    {
        dprintf("File too large: %d bytes\n", req->content_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File size must be less than " MAX_FILE_SIZE_STR "!");
        return ESP_FAIL;
    }

    while (!core2_web_lock())
        vPortYield();

    esp_err_t err;
    request = req;
    if ((err = core2_update_start(write_from_http, req->content_len)) != ESP_OK)
    {
        dprintf("core2_update_start failed - %d\n", err);

        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "core2_update_start failed");
        core2_web_unlock();
        return ESP_FAIL;
    }

    // core2_update_start_from_file("/sd/firmware.bin");

    xTaskCreate(restart_task, "restart_task", 1024, NULL, 0, NULL);

    httpd_resp_send(req, "Firmware OK", HTTPD_RESP_USE_STRLEN);
    core2_web_unlock();
    return ESP_OK;
}

esp_err_t firmware_post_handler(httpd_req_t *req)
{
    size_t len = req->content_len;
    dprintf("/firmware - Content length: %d\n", len);

    if (len > 3000000 || len <= 0)
    {
        httpd_resp_send_500(req);
        return ESP_OK;
    }

    FILE *f = core2_file_open("/sd/firmware.bin", "w");
    if (f == NULL)
    {
        httpd_resp_send_500(req);
        return ESP_OK;
    }

    size_t buffer_len = 4096;
    size_t read_len = 0;
    char *buffer = (char *)core2_malloc(buffer_len);
    dprintf("Writing /sd/firmware.bin\n");

    size_t written_len = 0;
    size_t remaining = len;

    while (true)
    {
        read_len = httpd_req_recv(req, buffer, min(remaining, buffer_len));

        if (read_len == HTTPD_SOCK_ERR_TIMEOUT)
            continue;

        if (read_len < 0)
            break;

        written_len += fwrite(buffer, 1, read_len, f);
        fflush(f);

        remaining -= read_len;

        if (read_len < buffer_len)
            break;
    }

    if (read_len < 0)
    {
        dprintf("httpd_req_recv - %d\n", read_len);
    }

    dprintf("Write complete, written %ld bytes\n", written_len);
    core2_file_close(f);
    core2_free(buffer);

    if (written_len != len)
    {
        dprintf("Written length != content length - FAIL\n");
        core2_file_delete("/sd/firmware.bin");

        httpd_resp_send_500(req);
        return ESP_OK;
    }

    core2_update_start_from_file("/sd/firmware.bin");

    httpd_resp_send(req, "Firmware OK", HTTPD_RESP_USE_STRLEN);
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

    params.printf = core2_printf;

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
        dprintf("Registering URI handlers\n");

        httpd_register_uri_handler(http_server, &root_handler);
        httpd_register_uri_handler(http_server, &shell_handler);
        httpd_register_uri_handler(http_server, &firmware_handler);

        dprintf("core2_http_start - OK\n");
        return true;
    }

    dprintf("core2_http_start - FAILED\n");
    return false;
}
