#include <core2.h>

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include <core2_update.h>
#include <esp_ghota.h>

static void ghota_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ghota_client_handle_t *client = (ghota_client_handle_t *)handler_args;
    ESP_LOGI(TAG, "Got Update Callback: %s", ghota_get_event_str(id));
    if (id == GHOTA_EVENT_START_STORAGE_UPDATE)
    {
        ESP_LOGI(TAG, "Starting storage update");

        // Mount storage here
    }
    else if (id == GHOTA_EVENT_FINISH_STORAGE_UPDATE)
    {
        ESP_LOGI(TAG, "Ending storage update");

        // Remount storage now?
    }
    else if (id == GHOTA_EVENT_FIRMWARE_UPDATE_PROGRESS)
    {
        /* display some progress with the firmware update */
        ESP_LOGI(TAG, "Firmware Update Progress: %d%%", *((int *)event_data));
    }
    else if (id == GHOTA_EVENT_STORAGE_UPDATE_PROGRESS)
    {
        /* display some progress with the spiffs partition update */
        ESP_LOGI(TAG, "Storage Update Progress: %d%%", *((int *)event_data));
    }
    (void)client;
    return;
}

void core2_update_start(void)
{
    /* initialize our ghota config */
    ghota_config_t ghconfig = {
        .filenamematch = "esp_ghota-esp32.bin",
        .storagenamematch = "storage-esp32.bin",
        .storagepartitionname = "storage",
        /* 1 minute as a example, but in production you should pick something larger (remember, Github has ratelimites on the API! )*/
        .updateInterval = 1,
    };

    /* initialize ghota. */
    ghota_client_handle_t *ghota_client = ghota_init(&ghconfig);
    if (ghota_client == NULL)
    {
        dprintf("ghota_client_init failed\n");
        return;
    }
    /* register for events relating to the update progress */
    esp_event_handler_register(GHOTA_EVENTS, ESP_EVENT_ANY_ID, &ghota_event_callback, ghota_client);
    ESP_ERROR_CHECK(ghota_start_update_task(ghota_client));

    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "This is where we do other things. Memory Dump Below to see the memory usage");
        ESP_LOGI(TAG, "Memory: Free %dKiB Low: %dKiB\n", (int)xPortGetFreeHeapSize() / 1024, (int)xPortGetMinimumEverFreeHeapSize() / 1024);
    }
}