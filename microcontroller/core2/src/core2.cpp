#include <core2.h>

#ifndef CORE2_WINDOWS
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <rtc.h>
#include <string.h>

#include <esp_netif.h>
#include <esp_sntp.h>

#include <USB.h>

#include <Wire.h>
#endif

#define printlogo dprintf

void core2_print(void *self, const char *str)
{
    dprintf("%s", str);
}

void core2_printf(void *self, const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);

    size_t req_len = vsnprintf(NULL, 0, fmt, argptr);
    char *buf = (char *)core2_malloc(req_len + 1);

    vsprintf(buf, fmt, argptr);

    va_end(argptr);

    ((core2_shell_func_params_t *)self)->print(self, buf);
    core2_free(buf);
}

void core2_init()
{
    printlogo("  _____             ___ \n");
    printlogo(" / ___/__  _______ |_  |\n");
    printlogo("/ /__/ _ \\/ __/ -_) __/ \n");
    printlogo("\\___/\\___/_/  \\__/____/ \n");
    printlogo("                        \n");

    dprintf("core2_init()\n");
    dprintf("Running ESP IDF " IDF_VER);

#ifdef CORE2_TDECK
    dprintf("Running on T-DECK, enabling board poweron\n");

    pinMode(BOARD_POWERON, OUTPUT);
    digitalWrite(BOARD_POWERON, HIGH);

    pinMode(BOARD_SDCARD_CS, OUTPUT);
    pinMode(RADIO_CS_PIN, OUTPUT);
    pinMode(BOARD_TFT_CS, OUTPUT);

    digitalWrite(BOARD_SDCARD_CS, HIGH);
    digitalWrite(RADIO_CS_PIN, HIGH);
    digitalWrite(BOARD_TFT_CS, HIGH);

    vTaskDelay(pdMS_TO_TICKS(100));
#endif

    dprintf("\n");
}

#define FEATURE_SPACE "   "
void core2_print_status()
{
#ifndef CORE2_WINDOWS
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

    dprintf("%lu MB %s flash\n", flash_size / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    uint32_t free_heap = esp_get_free_heap_size();
    dprintf("Free heap size: %ld bytes (%ld kb)\n", free_heap, free_heap / 1024);

    // TODO: spi ram not initialized
    // uint32_t spiram_size = esp_spiram_get_size();
    // dprintf("SPI RAM size: %ld bytes (%ld kb)\n", spiram_size, spiram_size / 1024);

    esp_reset_reason_t reason = esp_reset_reason();
    char resetreason[64];

    core2_resetreason_tostr(reason, resetreason, false);
    dprintf("%s - ", resetreason);

    core2_resetreason_tostr(reason, resetreason, true);
    dprintf("%s\n", resetreason);
#endif
}

#ifndef CORE2_WINDOWS
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

SemaphoreHandle_t general_lock = NULL;

bool core2_general_lock()
{
    if (general_lock == NULL)
        general_lock = core2_lock_create();

    return core2_lock_begin(general_lock);
}

bool core2_general_lock_force()
{
    while (!core2_general_lock())
        vPortYield();

    return true;
}

void core2_general_unlock()
{
    core2_lock_end(general_lock);
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
#define MAKE_CASE(err)                                                                                                 \
    case err:                                                                                                          \
        strcpy(buffer, #err);                                                                                          \
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
#define MAKE_CASE(err, descr)                                                                                          \
    case err:                                                                                                          \
        if (desc)                                                                                                      \
        {                                                                                                              \
            strcpy(buffer, descr);                                                                                     \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            strcpy(buffer, #err);                                                                                      \
        }                                                                                                              \
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
#endif

void *core2_malloc(size_t sz)
{
    void *ptr = malloc(sz);

    if (ptr == NULL)
    {
        eprintf("malloc(%zu) failed", sz);
        return NULL;
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

void *core2_malloc_ps(size_t sz)
{
    void *ptr = ps_malloc(sz);

    if (ptr == NULL)
    {
        eprintf("ps_malloc(%zu) failed", sz);
        return NULL;
    }

    memset(ptr, 0, sz);
    return ptr;
}

void *core2_realloc_ps(void *ptr, size_t sz)
{
    void *new_ptr = ps_realloc(ptr, sz);

    if (new_ptr == NULL)
    {
        eprintf("ps_realloc(%p ,%zu) failed", ptr, sz);
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

char *core2_string_copy_len(const char *str, size_t len)
{
    char *mem = (char *)core2_malloc(len + 1);
    memcpy(mem, str, len);
    return mem;
}

char *core2_string_copy(const char *str)
{
    return core2_string_copy_len(str, strlen(str));
}

void core2_strncpyz(char *dest, const char *src, int destsize)
{
    // dprintf("core2_strncpyz(\"%s\")\n", src);
    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

uint32_t core2_random()
{
#ifdef CORE2_WINDOWS
    return rand();
#else
    return esp_random();
#endif
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

void core2_sleep(int32_t ms)
{
#ifdef CORE2_WINDOWS
    Sleep(ms);
#else
    vTaskDelay(pdMS_TO_TICKS(ms));
#endif
}

void core2_main_impl(void *args)
{
    core2_main();

    for (;;)
    {
        core2_sleep(1000);
    }
}

void core2_wait_for_serial()
{
#if defined(CORE2_DOOR_CONTROLLER)
    return;
#endif

#ifndef CORE2_WINDOWS
    fflush(stdout);
    fpurge(stdin);

    Serial.begin(115200);

    while (!Serial)
        vTaskDelay(pdMS_TO_TICKS(100));

    Serial.flush();
#endif
}

void loop()
{
}

void setup2(void *args)
{
#if defined(CORE2_DOOR_CONTROLLER)
    core2_door_controller_setup();
#endif

    // vTaskDelay(pdMS_TO_TICKS(10000));
    core2_wait_for_serial();
    core2_init();

#ifdef CORE2_RUN_TESTS
    core2_array_run_tests();
    core2_json_test();

    dprintf("[TESTS] Done\n");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
#endif

#ifndef CORE2_WINDOWS
    core2_flash_init();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    core2_print_status();
    core2_gpio_init();

    // pinMode(SDCARD_PIN_CS, OUTPUT);
    // digitalWrite(SDCARD_PIN_CS, LOW);

    // core2_spi_init();

#ifndef CORE2_DISABLE_SDCARD
    sdmmc_host_t sdcard_host;
    if (core2_spi_create_sdmmc_host(&sdcard_host, SDCARD_PIN_MOSI, SDCARD_PIN_MISO, SDCARD_PIN_CLK))
    {
        if (!core2_filesystem_init(&sdcard_host, SDCARD_PIN_CS))
        {
            dprintf("SD Card failed to init, switching to AP mode\n");
            dprintf("TODO!\n");
        }
    }
#endif
#endif

    core2_shell_init();

#ifndef CORE2_DISABLE_WIFI
    core2_wifi_init();
#endif

#if defined(CORE2_TR_MOD)
    {
        core2_shell_register(
            "int0", [](core2_shell_func_params_t *params, int argc, char **argv) { core2_gpio_set_interrupt0(); });

        core2_mcp320x_init();
        // run_tests();

        core2_oled_init();
        // core2_wifi_init();
        core2_main_impl(NULL);
    }
#elif defined(CORE2_TDECK)
    {
        core2_tdeck_main();
    }
#elif defined(CORE2_CAN)
    {
        core2_can_main();
    }
#elif defined(CORE2_WINDOWS)
    {
        core2_main_impl();
    }
#elif defined(CORE2_DOOR_CONTROLLER)
    {
        core2_door_controller_main();
    }
#else
#error "Platform not defined"
#endif

    // core2_clock_init();

    // core2_wifi_yield_until_connected();

    // char cur_time[21];
    // core2_clock_time_now(cur_time);
    // dprintf("Current date time: %s\n", cur_time);

    // dprintf("init() done\n");

    /*
    #ifndef CORE2_WINDOWS
        xTaskCreate(core2_main_impl, "core2_main", 1024 * 32, NULL, 1, NULL);
        vTaskDelete(NULL);
    #endif*/
}

void setup()
{
    // xTaskCreate(setup2, "setup2", 1024 * 128, NULL, 1, NULL);
    // vTaskDelete(NULL);

    setup2(NULL);
}

#ifdef CORE2_WINDOWS
void main(int argc, const char **argv)
{
    setup();
}
#endif