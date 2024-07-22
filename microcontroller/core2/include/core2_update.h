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

    // Updates
    // =================================================================================================

    void core2_update_start();

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
}
#endif