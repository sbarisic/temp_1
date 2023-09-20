#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include "driver/sdmmc_host.h"

// Default defines
// =================================================================================================

#define CORE2_DEBUG
#define CORE2_DEBUG_WIFI

// Uncomment to disable compilation of modules
// #define CORE2_DISABLE_MCP320X
#define CORE2_DISABLE_OLED

// Uncomment to disable complilation and calling of test functions
#define CORE2_OMIT_TESTS

#ifdef CORE2_DEBUG
#define dprintf printf
#else
#define dprintf(...)
#endif

#define eprintf(...)         \
    do                       \
    {                        \
        printf("[ERROR] ");  \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while (0)

// #define CORE2_FILESYSTEM_VERBOSE_OUTPUT // Prints very long debug outputs to the output stream
#define CORE2_FILESYSTEM_SIMPLE_OUTPUT // Prints simple debug outputs to the output stream

// SD SPI pin config
// =================================================================================================

#if CONFIG_IDF_TARGET_ESP32

#define SDCARD_PIN_MOSI GPIO_NUM_26 // 
#define SDCARD_PIN_MISO GPIO_NUM_33 // 
#define SDCARD_PIN_CLK GPIO_NUM_32  // 
#define SDCARD_PIN_CS GPIO_NUM_23   // 

#elif CONFIG_IDF_TARGET_ESP32S3
#include "core2_tdeck.h"
#define ESP32_TDECK
void core2_main_tdeck();

#define SDCARD_PIN_MOSI BOARD_SPI_MOSI
#define SDCARD_PIN_MISO BOARD_SPI_MISO
#define SDCARD_PIN_CLK BOARD_SPI_SCK
#define SDCARD_PIN_CS BOARD_SDCARD_CS

#else
#error ESP32 target not defined
#endif

// ========= Entry Point ===========================================================================
void core2_main();

// MCP320X SPI pin config
// =================================================================================================

#define MCP320X_PIN_MOSI -1
#define MCP320X_PIN_MISO -1
#define MCP320X_PIN_CLK -1
#define MCP320X_PIN_CS -1

#define MCP320X_CS_CHANNEL1 GPIO_NUM_4
#define MCP320X_CS_CHANNEL2 GPIO_NUM_5

#define MCP320X_ADC_VREF 3311   // 3.3V Vref
#define MCP320X_ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

// GPIO config

#define INTERRUPT0_PIN GPIO_NUM_15

// Core
// =================================================================================================

void core2_init();
void core2_print_status();
uint32_t core2_random();

SemaphoreHandle_t core2_lock_create();
bool core2_lock_begin(SemaphoreHandle_t lock);
bool core2_lock_end(SemaphoreHandle_t lock);

xQueueHandle core2_queue_create(int count, int elementSize);
BaseType_t core2_queue_send(xQueueHandle q, const void *item);
BaseType_t core2_queue_receive(xQueueHandle q, void *buffer);
void core2_queue_reset(xQueueHandle q);

void core2_err_tostr(esp_err_t err, char *buffer);
void core2_resetreason_tostr(esp_reset_reason_t err, char *buffer, bool desc);

void *core2_malloc(size_t sz);
void *core2_realloc(void *ptr, size_t sz);
void core2_free(void *ptr);
char *core2_string_concat(const char *a, const char *b); // Should call core2_free() on result
bool core2_string_ends_with(const char *str, const char *end);

// OLED
// =================================================================================================

bool core2_oled_init();
void core2_oled_print(const char *txt);

// Wifi
// =================================================================================================

bool core2_wifi_init();
bool core2_wifi_isconnected();
IPAddress core2_wifi_getip();
void core2_wifi_yield_until_connected();
bool core2_wifi_ap_start();
bool core2_wifi_ap_stop();

// Clock
// =================================================================================================

bool core2_clock_init();
int32_t core2_clock_bootseconds();
int32_t core2_clock_seconds_since(int32_t lastTime);
void core2_clock_time_now(char *strftime_buf);
void core2_clock_time_fmt(char *strftime_buf, size_t max_size, const char *fmt);
void core2_clock_update_from_ntp();

// GPIO
// =================================================================================================

bool core2_gpio_init();
bool core2_gpio_get_interrupt0();
bool core2_gpio_set_interrupt0();
void core2_gpio_clear_interrupt0();
bool core2_gpio_enable_interrupt0(bool enable);

// Flash
// =================================================================================================

bool core2_flash_init();

// Filesystem
// =================================================================================================

typedef void (*onFileFoundFn)(const char *full_name, const char *file_name);

bool core2_filesystem_init(sdmmc_host_t *host, int CS);
FILE *core2_file_open(const char *filename, const char *type = NULL);
bool core2_file_close(FILE *f);
bool core2_file_move(const char *full_file_path, const char *new_directory);
bool core2_file_write(const char *filename, const char *data, size_t len);
bool core2_file_append(const char *filename, const char *data, size_t len);
bool core2_file_mkdir(const char *dirname, mode_t mode = 0);
void core2_file_list(const char *dirname, onFileFoundFn onFileFound);
bool core2_file_write_timesuffix(const char *filename, const char *data, size_t len);

// MCP320X ADC
// =================================================================================================

typedef enum
{
    CORE2_ADC_CH1 = 1 << 0,
    CORE2_ADC_CH2 = 1 << 1,
} core2_adc_channel_t;

bool core2_mcp320x_init();
bool core2_adc_lock();
void core2_adc_unlock(bool was_locked);
// void core2_adc_read(float *Volt1, float *Volt2);
void core2_adc_read_ex(float *VoltArray, float *Factors, core2_adc_channel_t Ch, bool UseLock);
void core2_adc_read(float *Volt1, float *Volt2);

// SPI
// =================================================================================================

bool core2_spi_init();
bool core2_spi_create(sdmmc_host_t *host, int MOSI, int MISO, int CLK);

// JSON
// =================================================================================================

typedef enum
{
    CORE2_JSON_INVALID = 0,
    CORE2_JSON_FLOAT = 1,
    CORE2_JSON_STRING = 2,
    CORE2_JSON_FLOAT_ARRAY = 3,
    CORE2_JSON_INT = 4,
    CORE2_JSON_FLOAT_DEC2 = 5,
    CORE2_JSON_FLOAT_ARRAY_DEC2 = 6,
} core2_json_fieldtype_t;

bool core2_json_init();
void core2_json_begin();
void core2_json_add_field(const char *field_name, void *data, size_t len, core2_json_fieldtype_t data_type);
void core2_json_add_field_string(const char *field_name, const char *str);
void core2_json_add_field_int(const char *field_name, int num);
void core2_json_end(char **dest_buffer, size_t *json_length);
void core2_json_serialize(char **dest_buffer, size_t *json_length);

// Web
bool core2_web_json_post(const char *server_name, const char *json_txt, size_t json_txt_len);

// Shell & Telnet
// =================================================================================================

typedef void (*core2_shell_func)();

typedef struct
{
    const char *name;
    core2_shell_func func;
} core2_shell_cmd_t;

void core2_shell_register(const char *func_name, core2_shell_func func);
void core2_shell_init();