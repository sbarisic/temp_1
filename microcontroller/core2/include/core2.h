#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CORE2_WINDOWS
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef CORE2_WINDOWS
	// Windows specifics here
#define IDF_VER "Windows"

#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")


	typedef int32_t IPAddress;
	typedef int32_t sdmmc_host_t;
	typedef int32_t mode_t;
#else
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "driver/sdmmc_host.h"
#include <WiFi.h>
#endif


#ifdef CORE2_TDECK
#include "core2_tdeck.h"
#endif

	// Default defines
	// =================================================================================================

#define CORE2_DEBUG
#define CORE2_DEBUG_WIFI
// #define CORE2_DEBUG_ARRAY

// #define CORE2_AP_MODE_ONLY // Start wifi in access mode only

// Uncomment to disable compilation of modules
#define CORE2_DISABLE_MCP320X
#define CORE2_DISABLE_OLED

// Uncomment to run tests only
// #define CORE2_RUN_TESTS

#ifdef CORE2_DEBUG
#define dprintf printf
#else
#define dprintf(...)
#endif

#define eprintf(...)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("[ERROR] ");                                                                                            \
        printf(__VA_ARGS__);                                                                                           \
        printf("\n");                                                                                                  \
    } while (0)

// #define CORE2_FILESYSTEM_VERBOSE_OUTPUT // Prints very long debug outputs to the output stream
#define CORE2_FILESYSTEM_SIMPLE_OUTPUT // Prints simple debug outputs to the output stream

// SD SPI pin config
// =================================================================================================

#if defined(CORE2_TDECK)
#define SDCARD_PIN_MOSI BOARD_SPI_MOSI
#define SDCARD_PIN_MISO BOARD_SPI_MISO
#define SDCARD_PIN_CLK BOARD_SPI_SCK
#define SDCARD_PIN_CS BOARD_SDCARD_CS
#elif defined(CORE2_WINDOWS)
#define SDCARD_PIN_MOSI 0 
#define SDCARD_PIN_MISO 0 
#define SDCARD_PIN_CLK 0  
#define SDCARD_PIN_CS 0   
#else
#define SDCARD_PIN_MOSI GPIO_NUM_26 //
#define SDCARD_PIN_MISO GPIO_NUM_33 //
#define SDCARD_PIN_CLK GPIO_NUM_32  //
#define SDCARD_PIN_CS GPIO_NUM_23   //
#endif

// ========= Entry Point ===========================================================================
	void core2_main();

	// MCP320X SPI pin config
	// =================================================================================================

#define MCP320X_PIN_MOSI -1
#define MCP320X_PIN_MISO -1
#define MCP320X_PIN_CLK -1
#define MCP320X_PIN_CS -1

#if defined(CORE2_WINDOWS)
#define MCP320X_CS_CHANNEL1 0
#define MCP320X_CS_CHANNEL2 0
#else
#define MCP320X_CS_CHANNEL1 GPIO_NUM_4
#define MCP320X_CS_CHANNEL2 GPIO_NUM_5
#endif

#define MCP320X_ADC_VREF 3311   // 3.3V Vref
#define MCP320X_ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

// GPIO config

#define INTERRUPT0_PIN GPIO_NUM_15

// Core
// =================================================================================================

	void core2_init();
	void core2_print_status();
	uint32_t core2_random();
	void core2_sleep(int32_t ms);

#ifndef CORE2_WINDOWS
	SemaphoreHandle_t core2_lock_create();
	bool core2_lock_begin(SemaphoreHandle_t lock);
	bool core2_lock_end(SemaphoreHandle_t lock);

	xQueueHandle core2_queue_create(int count, int elementSize);
	BaseType_t core2_queue_send(xQueueHandle q, const void* item);
	BaseType_t core2_queue_receive(xQueueHandle q, void* buffer);
	void core2_queue_reset(xQueueHandle q);

	void core2_err_tostr(esp_err_t err, char* buffer);
	void core2_resetreason_tostr(esp_reset_reason_t err, char* buffer, bool desc);
#endif

	void* core2_malloc(size_t sz);
	void* core2_realloc(void* ptr, size_t sz);
	void core2_free(void* ptr);
	char* core2_string_concat(const char* a, const char* b); // Should call core2_free() on result
	bool core2_string_ends_with(const char* str, const char* end);
	char* core2_string_copy_len(const char* str, size_t len);
	char* core2_string_copy(const char* str);
	void core2_strncpyz(char* dest, const char* src, int destsize);

	typedef struct
	{
		void* memory;
		size_t element_size;
		size_t length;
	} core2_array_t;

	core2_array_t* core2_array_create(size_t element_size);
	void core2_array_delete(core2_array_t* array);
	void core2_array_get(core2_array_t* array, int index, void* element_ptr);
	void core2_array_set(core2_array_t* array, int index, void* element_ptr);
	void core2_array_insert_end(core2_array_t* array, void* element_ptr);
	void core2_array_run_tests();

	// OLED
	// =================================================================================================

	bool core2_oled_init();
	void core2_oled_print(const char* txt);

	// Shell
	// =================================================================================================

#define MAX_STRING_TOKENS 2048
#define BIG_INFO_STRING 2048

	typedef struct tokenize_info
	{
		int cmd_argc;
		char* cmd_argv[MAX_STRING_TOKENS];                       // points into cmd_tokenized
		char cmd_tokenized[BIG_INFO_STRING + MAX_STRING_TOKENS]; // will have 0 bytes inserted
		char cmd_cmd[BIG_INFO_STRING];                           // the original command we received (no token processing)
	} tokenize_info_t;

	typedef void (*core2_shell_print_func)(void* self, const char* str);

	typedef struct
	{
		core2_shell_print_func print;
		void* ud1;
		void* ud2;
		void* ud3;
	} core2_shell_func_params_t;

	typedef void (*core2_shell_func)(core2_shell_func_params_t* params, int argc, char** argv);

	typedef struct
	{
		const char* name;
		core2_shell_func func;
	} core2_shell_cmd_t;

	typedef enum
	{
		CORE2_CVAR_INT32,
		CORE2_CVAR_STRING
	} core2_cvar_type;

	typedef struct
	{
		const char* name;
		core2_cvar_type var_type;
		void* var_ptr;
	} core2_shell_cvar_t;

	void core2_shell_register(const char* func_name, core2_shell_func func);
	bool core2_shell_invoke(const char* full_command, core2_shell_func_params_t* params);
	void core2_shell_init();

	void core2_shell_cvar_register(core2_shell_cvar_t* cvar, const char* var_name, void* var_ptr, core2_cvar_type var_type);
	size_t core2_shell_cvar_count();
	core2_shell_cvar_t* core2_shell_cvar_get(int idx);
	core2_shell_cvar_t* core2_shell_cvar_find(const char* var_name);
	void core2_shell_save_cvars();
	void core2_shell_load_cvars();

	// Wifi
	// =================================================================================================

	bool core2_wifi_init();
	bool core2_wifi_isconnected();
	IPAddress core2_wifi_getip();
	void core2_wifi_yield_until_connected();
	bool core2_wifi_ap_start();
	bool core2_wifi_ap_stop();
	bool core2_wifi_try_connect(const char* ssid, const char* pass);

	// Clock
	// =================================================================================================

	bool core2_clock_init();
	int32_t core2_clock_bootseconds();
	int32_t core2_clock_seconds_since(int32_t lastTime);
	void core2_clock_time_now(char* strftime_buf);
	void core2_clock_time_fmt(char* strftime_buf, size_t max_size, const char* fmt);
	void core2_clock_update_from_ntp();

	// GPIO
	// =================================================================================================

	bool core2_gpio_init();
	bool core2_gpio_get_interrupt0();
	bool core2_gpio_set_interrupt0();
	void core2_gpio_clear_interrupt0();
	bool core2_gpio_enable_interrupt0(bool enable);
	int core2_gpio_hall_read();

	// Flash
	// =================================================================================================

	bool core2_flash_init();
	void core2_flash_cvar_store(core2_shell_cvar_t* cvar);
	void core2_flash_cvar_load(core2_shell_cvar_t* cvar);

	// Filesystem
	// =================================================================================================

	typedef void (*onFileFoundFn)(const char* full_name, const char* file_name);

	bool core2_filesystem_init(sdmmc_host_t* host, int CS);
	FILE* core2_file_open(const char* filename, const char* type);
	bool core2_file_close(FILE* f);
	bool core2_file_move(const char* full_file_path, const char* new_directory);
	bool core2_file_write(const char* filename, const char* data, size_t len);
	bool core2_file_append(const char* filename, const char* data, size_t len);
	bool core2_file_mkdir(const char* dirname, mode_t mode);
	void core2_file_list(const char* dirname, onFileFoundFn onFileFound);
	bool core2_file_write_timesuffix(const char* filename, const char* data, size_t len);
	bool core2_file_exists(const char* filename);
	size_t core2_file_length(FILE* f);
	void* core2_file_read_all(const char* filename, size_t* len);

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
	void core2_adc_read_ex(float* VoltArray, float* Factors, core2_adc_channel_t Ch, bool UseLock);
	void core2_adc_read(float* Volt1, float* Volt2);

	// SPI
	// =================================================================================================

	bool core2_spi_init();
	bool core2_spi_create_sdmmc_host(sdmmc_host_t* host, int MOSI, int MISO, int CLK);

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

		// core2_json_add
		CORE2_JSON_BEGIN_ARRAY = 100,
		CORE2_JSON_END_ARRAY = 101,
		CORE2_JSON_BEGIN_OBJECT = 102,
		CORE2_JSON_END_OBJECT = 103
	} core2_json_fieldtype_t;

	typedef struct
	{
		char* buffer;
		size_t buffer_len;
		size_t buffer_content_len;
		bool need_comma;
	} core2_json_t;

	void core2_json_test();
	core2_json_t* core2_json_create();
	void core2_json_delete(core2_json_t* json);

	void core2_json_add(core2_json_t* json, core2_json_fieldtype_t data_type);
	void core2_json_begin_field(core2_json_t* json, const char* field_name, core2_json_fieldtype_t data_type);

	void core2_json_add_field(core2_json_t* json, const char* field_name, const void* data, size_t len,
		core2_json_fieldtype_t data_type);
	void core2_json_add_field_string(core2_json_t* json, const char* field_name, const char* str);
	void core2_json_add_field_int(core2_json_t* json, const char* field_name, int num);

	void core2_json_serialize(core2_json_t* json, char** dest_buffer, size_t* json_length);

	// Web
	// =================================================================================================

	bool core2_web_json_post(const char* server_name, const char* json_txt, size_t json_txt_len);

	// HTTP
	// =================================================================================================

	bool core2_http_start();

	// Updates
	// =================================================================================================

	void core2_update_start();


#ifdef __cplusplus
}
#endif