#include <core2.h>
#include <core2_variables.h>

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <core2_update.h>

#include "esp_ota_ops.h"

#include <WiFi.h>
#include <HTTPClient.h>

#define CORE2_WEB_USER_AGENT "Core2_ESP32"

esp_ota_handle_t update_handle;
esp_err_t ota_write(const void *data, size_t size)
{
    // dprintf("esp_ota_write(update_handle, data, %d)", size);

    esp_err_t err = esp_ota_write(update_handle, data, size);
    core2_sleep(1);
    // dprintf(" - %d\n", err);

    if (err != ESP_OK)
    {
        dprintf("esp_ota_write(update_handle, data, %d)", size);
        dprintf(" - %d\n", err);
    }

    return err;
}

FILE *f = NULL;
size_t len;

esp_err_t core2_write_flash_from_file(ota_write_func ota_write)
{
    size_t chunk_size = 4096;
    void *buffer = core2_malloc(chunk_size);

    int counter = 0;
    int last_print_percent = -1;

    while (true)
    {
        size_t read = fread(buffer, 1, chunk_size, f);

        int percent = (counter * chunk_size * 100) / len;
        counter++;

        if (last_print_percent != percent)
        {
            last_print_percent = percent;
            dprintf("ota_write(...) (%d %%)\n", percent);
        }

        ota_write(buffer, read);

        if (read < chunk_size)
        {
            dprintf("ota_write(...) (100 %%)\n");
            break;
        }
    }

    core2_free(buffer);
    return ESP_OK;
}

void core2_update_start_from_file(const char *file_name)
{
    f = core2_file_open(file_name, "r");
    if (f == NULL)
    {
        dprintf("File not found: %s\n", file_name);
        return;
    }

    len = core2_file_length(f);
    core2_update_start(core2_write_flash_from_file, len);

    core2_file_close(f);
    f = NULL;

    core2_file_delete(file_name);
}

void core2_update_start_from_server(const char *current_version)
{
    dprintf("core2_update_start_from_server()\n");

    const char *url_info = core2_shell_cvar_get_string_ex(cvar_firmware_info_url);
    const char *url_firm = core2_shell_cvar_get_string_ex(cvar_firmware_bin_url);
    dprintf("URL Info: %s\n", url_info);
    dprintf("URL Firm: %s\n", url_firm);

    HTTPClient http;
    http.begin(url_info);
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("User-Agent", CORE2_WEB_USER_AGENT);

    int getResult = http.GET();
    dprintf("%d\n", getResult);

    if (getResult == HTTP_CODE_OK)
    {
        String resp = http.getString();
        dprintf("Server version: '%s'\n", resp.c_str());
        dprintf("Current version: '%s'\n", current_version);

        if (strcmp(current_version, resp.c_str()) != 0)
        {
            dprintf("Updating\n");
            http.end();

            http.begin(url_firm);
            http.addHeader("Content-Type", "application/octet-stream");
            http.addHeader("User-Agent", CORE2_WEB_USER_AGENT);

            getResult = http.GET();
            dprintf("%d\n", getResult);

            if (getResult == HTTP_CODE_OK)
            {
                WiFiClient &cli = http.getStream();

                f = core2_file_open("/sd/firmware.bin", "w");
                if (f == NULL)
                {
                    dprintf("Unable to open /sd/firmware.bin for writing\n");
                    return;
                }

                size_t buffer_size = 4096;
                void *buffer = core2_malloc(buffer_size);
                dprintf("Writing /sd/firmware.bin\n");

                while (true)
                {
                    size_t bytes_read = cli.readBytes((char *)buffer, buffer_size);
                    fwrite(buffer, 1, bytes_read, f);
                    core2_sleep(1);

                    if (bytes_read < buffer_size)
                    {
                        break;
                    }
                }

                core2_file_close(f);
                http.end();
                core2_free(buffer);
                dprintf("Write complete\n");

                core2_update_start_from_file("/sd/firmware.bin");
            }
        }
        else
        {
            dprintf("Skipping update\n");
        }
    }
}

esp_err_t core2_update_start(core2_write_flash write_func, size_t size)
{
    dprintf("Starting flash (%ld bytes)\n", size);

    const esp_partition_t *running = esp_ota_get_running_partition();
    dprintf("Running partition: %s\n", running->label);

    if (!running)
    {
        dprintf("esp_ota_get_running_partition() - FAIL\n");
        return ESP_FAIL;
    }

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(running);
    dprintf("Update partition: %s\n", update_partition->label);

    if (update_partition == NULL)
    {
        dprintf("esp_ota_get_next_update_partition(running) returned NULL\n");
        return ESP_FAIL;
    }

    dprintf("esp_ota_begin\n");
    esp_err_t err = esp_ota_begin(update_partition, size, &update_handle);

    if (err != ESP_OK)
    {
        dprintf("esp_ota_begin(update_partition, size, &update_handle) - %d\n", err);
        return err;
    }

    dprintf("write_func(ota_write)\n");
    err = write_func(ota_write);

    if (err != ESP_OK)
    {
        dprintf("write_func(ota_write) - %d\n", err);
        return err;
    }

    err = esp_ota_end(update_handle);

    if (err != ESP_OK)
    {
        dprintf("esp_ota_end(update_handle) - %d\n", err);
        return err;
    }

    err = esp_ota_set_boot_partition(update_partition);

    if (err != ESP_OK)
    {
        dprintf("esp_ota_set_boot_partition(update_partition) - %d\n", err);
        return err;
    }

    dprintf("Flash complete\n");
    return ESP_OK;
}