#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

#ifdef CORE2_TDECK
#define SDCARD_PIN_MOSI BOARD_SPI_MOSI
#define SDCARD_PIN_MISO BOARD_SPI_MISO
#define SDCARD_PIN_CLK BOARD_SPI_SCK
#define SDCARD_PIN_CS BOARD_SDCARD_CS
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

#define MCP320X_CS_CHANNEL1 GPIO_NUM_4
#define MCP320X_CS_CHANNEL2 GPIO_NUM_5

#define MCP320X_ADC_VREF 3311   // 3.3V Vref
#define MCP320X_ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

// GPIO config

#define INTERRUPT0_PIN GPIO_NUM_15

// Core
// =================================================================================================

void* core2_malloc(size_t sz);
void* core2_realloc(void* ptr, size_t sz);
void core2_free(void* ptr);
char* core2_string_concat(const char* a, const char* b); // Should call core2_free() on result
bool core2_string_ends_with(const char* str, const char* end);
char* core2_string_copy_len(const char* str, size_t len);
char* core2_string_copy(const char* str);
void core2_strncpyz(char* dest, const char* src, int destsize);

typedef struct {
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

// Shell
// =================================================================================================

#define MAX_STRING_TOKENS 2048
#define BIG_INFO_STRING 2048

typedef struct tokenize_info {
    int cmd_argc;
    char* cmd_argv[MAX_STRING_TOKENS];                       // points into cmd_tokenized
    char cmd_tokenized[BIG_INFO_STRING + MAX_STRING_TOKENS]; // will have 0 bytes inserted
    char cmd_cmd[BIG_INFO_STRING];                           // the original command we received (no token processing)
} tokenize_info_t;

typedef void (*core2_shell_print_func)(void* self, const char* str);

typedef struct {
    core2_shell_print_func print;
    void* ud1;
    void* ud2;
    void* ud3;
} core2_shell_func_params_t;

typedef void (*core2_shell_func)(core2_shell_func_params_t* params, int argc, char** argv);

typedef struct {
    const char* name;
    core2_shell_func func;
} core2_shell_cmd_t;

typedef enum {
    CORE2_CVAR_INT32,
    CORE2_CVAR_STRING
} core2_cvar_type;

typedef struct {
    const char* name;
    core2_cvar_type var_type;
    void* var_ptr;
} core2_shell_cvar_t;

core2_shell_cmd_t* core2_shell_register(const char* func_name, core2_shell_func func);
bool core2_shell_invoke(const char* full_command, core2_shell_func_params_t* params);
void core2_shell_init();

void core2_shell_cvar_register(core2_shell_cvar_t* cvar, const char* var_name, void* var_ptr, core2_cvar_type var_type);
size_t core2_shell_cvar_count();
core2_shell_cvar_t* core2_shell_cvar_get(int idx);
core2_shell_cvar_t* core2_shell_cvar_find(const char* var_name);
void core2_shell_save_cvars();
void core2_shell_load_cvars();
