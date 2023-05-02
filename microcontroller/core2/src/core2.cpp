#include <core2.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <nvs_flash.h>

void core2_init()
{
    dprintf("core2_init() BEGIN\n");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        dprintf("Doing nvs_flash_erase()\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    dprintf("core2_init() END\n");
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