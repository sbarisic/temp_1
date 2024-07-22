#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CORE2_WINDOWS
#endif

#define CONFIG_MAX_FILENAME_LEN 128
#define CONFIG_MAX_URL_LEN 256
#define CONFIG_GITHUB_HOSTNAME "api.github.com"
#define CONFIG_GITHUB_OWNER "Collective-Code-Technologies"
#define CONFIG_GITHUB_REPO "firmware"

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
extern "C"
{
#endif

    typedef enum
    {
        CORE2_EVENT_OTA_BEGIN,
        CORE2_EVENT_OTA_WRITE,
        CORE2_EVENT_OTA_END
    } core2_update_event_t;

    typedef void (*core2_update_callback_func)(core2_update_event_t e);

    typedef esp_err_t (*ota_write_func)(const void *data, size_t size);
    typedef esp_err_t (*core2_write_flash)(ota_write_func ota_write);

    // Updates
    // =================================================================================================

    void core2_update_callback(core2_update_callback_func func);

    void core2_update_start_from_file(const char *file_name);
    void core2_update_start_from_server(const char *current_version);
    esp_err_t core2_update_start(core2_write_flash write_func, size_t size);

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
}
#endif