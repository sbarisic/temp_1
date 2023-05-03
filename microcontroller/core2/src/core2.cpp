#include <core2.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <nvs_flash.h>
#include <rtc.h>

#include <esp_netif.h>
#include <esp_sntp.h>

void core2_init()
{
    dprintf("core2_init()\n");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        dprintf("Doing nvs_flash_erase()\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
}

void core2_print_status()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;

    esp_chip_info(&chip_info);

    dprintf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;

    dprintf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        dprintf("Get flash size failed");
        return;
    }

    dprintf("%luMB %s flash\n", flash_size / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    dprintf("Minimum free heap size: %ld bytes\n", esp_get_minimum_free_heap_size());
}

SemaphoreHandle_t core2_lock_create()
{
    SemaphoreHandle_t lock = xSemaphoreCreateBinary();
    xSemaphoreGive(lock);
    return lock;
}

void core2_lock_begin(SemaphoreHandle_t lock)
{
    xSemaphoreTake(lock, portMAX_DELAY);
}

void core2_lock_end(SemaphoreHandle_t lock)
{
    xSemaphoreGive(lock);
}