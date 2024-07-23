#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CORE2_WINDOWS
#endif

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
extern "C"
{
#endif

    void core2_server_udp_start();

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
}
#endif