#include <core2.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <nvs_flash.h>
#include <rtc.h>

#include <esp_netif.h>
#include <esp_sntp.h>
#include <string.h>

#include <USB.h>

#define printlogo dprintf

void core2_init()
{
    printlogo("  _____             ___ \n");
    printlogo(" / ___/__  _______ |_  |\n");
    printlogo("/ /__/ _ \\/ __/ -_) __/ \n");
    printlogo("\\___/\\___/_/  \\__/____/ \n");
    printlogo("                        \n");

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

#define FEATURE_SPACE "   "
void core2_print_status()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;

    esp_chip_info(&chip_info);

    dprintf("This is %s chip with %d CPU core(s), features:\n", CONFIG_IDF_TARGET, chip_info.cores);

    if (chip_info.features & CHIP_FEATURE_WIFI_BGN)
        dprintf(FEATURE_SPACE "2.4GHz WiFi\n");

    if (chip_info.features & CHIP_FEATURE_BLE)
        dprintf(FEATURE_SPACE "Bluetooth LE\n");

    if (chip_info.features & CHIP_FEATURE_BT)
        dprintf(FEATURE_SPACE "Bluetooth Classic\n");

    if (chip_info.features & CHIP_FEATURE_IEEE802154)
        dprintf(FEATURE_SPACE "IEEE 802.15.4\n");

    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
        dprintf(FEATURE_SPACE "Embedded PSRAM\n");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;

    dprintf("Silicon revision v%d.%d\n", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        dprintf("Get flash size failed");
        return;
    }

    dprintf("%lu MB %s flash\n", flash_size / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    uint32_t free_heap = esp_get_minimum_free_heap_size();
    dprintf("Minimum free heap size: %ld bytes (%ld kb)\n", free_heap, free_heap / 1024);

    esp_reset_reason_t reason = esp_reset_reason();
    char resetreason[64];

    core2_resetreason_tostr(reason, resetreason, false);
    dprintf("%s - ", resetreason);

    core2_resetreason_tostr(reason, resetreason, true);
    dprintf("%s\n", resetreason);
}

SemaphoreHandle_t core2_lock_create()
{
    SemaphoreHandle_t lock = xSemaphoreCreateBinary();
    xSemaphoreGive(lock);
    return lock;
}

bool core2_lock_begin(SemaphoreHandle_t lock)
{
    if (lock == NULL)
    {
        dprintf("core2_lock_begin() received NULL\n");
        return false;
    }

    return xSemaphoreTake(lock, portMAX_DELAY);
}

bool core2_lock_end(SemaphoreHandle_t lock)
{
    if (lock == NULL)
    {
        dprintf("core2_lock_end() received NULL\n");
        return false;
    }

    return xSemaphoreGive(lock);
}

xQueueHandle core2_queue_create(int count, int elementSize)
{
    return xQueueCreate(count, elementSize);
}

BaseType_t core2_queue_send(xQueueHandle q, const void *item)
{
    if (xPortInIsrContext())
    {
        return xQueueSendFromISR(q, item, NULL);
    }
    else
    {
        return xQueueSend(q, item, portMAX_DELAY);
    }
}

BaseType_t core2_queue_receive(xQueueHandle q, void *buffer)
{
    if (xPortInIsrContext())
    {
        return xQueueReceiveFromISR(q, buffer, NULL);
    }
    else
    {
        return xQueueReceive(q, buffer, portMAX_DELAY);
    }
}

void core2_queue_reset(xQueueHandle q)
{
    xQueueReset(q);
}

// @brief Expects 30 byte buffer
void core2_err_tostr(esp_err_t err, char *buffer)
{
#define MAKE_CASE(err)        \
    case err:                 \
        strcpy(buffer, #err); \
        break

    switch (err)
    {
        MAKE_CASE(ESP_OK);
        MAKE_CASE(ESP_FAIL);
        MAKE_CASE(ESP_ERR_NO_MEM);
        MAKE_CASE(ESP_ERR_INVALID_ARG);
        MAKE_CASE(ESP_ERR_INVALID_STATE);
        MAKE_CASE(ESP_ERR_INVALID_SIZE);
        MAKE_CASE(ESP_ERR_NOT_FOUND);
        MAKE_CASE(ESP_ERR_NOT_SUPPORTED);
        MAKE_CASE(ESP_ERR_TIMEOUT);

        MAKE_CASE(ESP_ERR_INVALID_RESPONSE);
        MAKE_CASE(ESP_ERR_INVALID_CRC);
        MAKE_CASE(ESP_ERR_INVALID_VERSION);
        MAKE_CASE(ESP_ERR_INVALID_MAC);
        MAKE_CASE(ESP_ERR_NOT_FINISHED);

        MAKE_CASE(ESP_ERR_WIFI_BASE);
        MAKE_CASE(ESP_ERR_MESH_BASE);
        MAKE_CASE(ESP_ERR_FLASH_BASE);
        MAKE_CASE(ESP_ERR_HW_CRYPTO_BASE);
        MAKE_CASE(ESP_ERR_MEMPROT_BASE);

    default:
        strcpy(buffer, "UNKNOWN");
        break;
    }
}

// @brief Expects 64 byte buffer
void core2_resetreason_tostr(esp_reset_reason_t err, char *buffer, bool desc)
{
#undef MAKE_CASE
#define MAKE_CASE(err, descr)      \
    case err:                      \
        if (desc)                  \
        {                          \
            strcpy(buffer, descr); \
        }                          \
        else                       \
        {                          \
            strcpy(buffer, #err);  \
        }                          \
        break

    switch (err)
    {
        MAKE_CASE(ESP_RST_UNKNOWN, "Reset reason can not be determined");
        MAKE_CASE(ESP_RST_POWERON, "Reset due to power-on event");
        MAKE_CASE(ESP_RST_EXT, "Reset by external pin (not applicable for ESP32)");
        MAKE_CASE(ESP_RST_SW, "Software reset via esp_restart");
        MAKE_CASE(ESP_RST_PANIC, "Software reset due to exception/panic");
        MAKE_CASE(ESP_RST_INT_WDT, "Reset (software or hardware) due to interrupt watchdog");
        MAKE_CASE(ESP_RST_TASK_WDT, "Reset due to task watchdog");
        MAKE_CASE(ESP_RST_WDT, "Reset due to other watchdogs");
        MAKE_CASE(ESP_RST_DEEPSLEEP, "Reset after exiting deep sleep mode");
        MAKE_CASE(ESP_RST_BROWNOUT, "Brownout reset (software or hardware)");
        MAKE_CASE(ESP_RST_SDIO, "Reset over SDIO");

    default:
        strcpy(buffer, "UNKNOWN");
        break;
    }
}

void *core2_malloc(size_t sz)
{
    void *ptr = malloc(sz);

    if (ptr == NULL)
    {
        eprintf("malloc(%zu) failed", sz);
    }

    memset(ptr, 0, sz);
    return ptr;
}

void *core2_realloc(void *ptr, size_t sz)
{
    void *new_ptr = realloc(ptr, sz);

    if (new_ptr == NULL)
    {
        eprintf("realloc(%p ,%zu) failed", ptr, sz);
    }

    return new_ptr;
}

void core2_free(void *ptr)
{
    free(ptr);
}

// Resulting string needs to be free'd after use
char *core2_string_concat(const char *a, const char *b)
{
    size_t len = strlen(a) + strlen(b) + 1;
    char *res = (char *)core2_malloc(len);

    if (res == NULL)
        return NULL;

    memset(res, 0, len);
    strcpy(res, a);
    strcat(res, b);

    return res;
}

uint32_t core2_random()
{
    return esp_random();
}

bool core2_string_ends_with(const char *str, const char *end)
{
    if (str == NULL || end == NULL)
        return false;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(end);

    if (lensuffix > lenstr)
        return false;

    return strncmp(str + lenstr - lensuffix, end, lensuffix) == 0;
}

void core2_main_impl(void *args)
{
    core2_main();

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void core2_wait_for_serial()
{
    fflush(stdout);
    fpurge(stdin);

    Serial.begin(115200);

    while (!Serial)
        vTaskDelay(pdMS_TO_TICKS(100));

    Serial.flush();
}

void loop()
{
}

void setup()
{
    // vTaskDelay(pdMS_TO_TICKS(10000));
    core2_wait_for_serial();

    core2_init();
    core2_print_status();

#ifdef ESP32_TDECK
    core2_main_tdeck();
#else

    // pinMode(SDCARD_PIN_CS, OUTPUT);
    // digitalWrite(SDCARD_PIN_CS, LOW);

    core2_spi_init();
    sdmmc_host_t sdcard_host;
    if (core2_spi_create(&sdcard_host, SDCARD_PIN_MOSI, SDCARD_PIN_MISO, SDCARD_PIN_CLK))
    {
        core2_filesystem_init(&sdcard_host, SDCARD_PIN_CS);
    }

    core2_mcp320x_init();
    // run_tests();

    core2_flash_init();
    core2_gpio_init();
    core2_oled_init();
    core2_wifi_init();
    core2_clock_init();
    core2_json_init();
    core2_shell_init();

    core2_wifi_yield_until_connected();
    dprintf("init() done\n");

    char cur_time[21];
    core2_clock_time_now(cur_time);
    dprintf("Current date time: %s\n", cur_time);

    /*char filename[30];
    core2_clock_time_fmt(filename, sizeof(filename), "/sd/boot_%d%m%Y_%H%M%S.txt");

    const char *Text = "Hello ESP32 World!\n";
    core2_file_write(filename, Text, strlen(Text));*/

    core2_shell_register("int0", []()
                         { core2_gpio_set_interrupt0(); });

    core2_shell_register("esp_restart", esp_restart);
    xTaskCreate(core2_main_impl, "core2_main", 1024 * 16, NULL, 1, NULL);
#endif

    // vTaskDelay(pdMS_TO_TICKS(1000 * 20));
    // core2_wifi_ap_stop();

    // Stop arduino task, job done
    vTaskDelete(NULL);
}