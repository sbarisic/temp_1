#pragma once
#include <core2.h>

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CORE2_WINDOWS
#endif

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
#define EXTERNC extern "C"

extern "C"
{
#else
#define EXTERNC extern 
#endif

#if defined(CORE2_VARIABLES_IMPL)
#define CORE2_EXTERN
#else
#define CORE2_EXTERN EXTERNC
#endif


// User variables
CORE2_EXTERN core2_shell_cvar_t *cvar_vehicle_name;
CORE2_EXTERN core2_shell_cvar_t *cvar_vehicle_number;
CORE2_EXTERN core2_shell_cvar_t *cvar_network_ssid;
CORE2_EXTERN core2_shell_cvar_t *cvar_network_password;


CORE2_EXTERN core2_shell_cvar_t *cvar_ap_ssid;
CORE2_EXTERN core2_shell_cvar_t *cvar_ap_password;
CORE2_EXTERN core2_shell_cvar_t *cvar_host;
CORE2_EXTERN core2_shell_cvar_t *cvar_port;
CORE2_EXTERN core2_shell_cvar_t *cvar_api_key;
CORE2_EXTERN core2_shell_cvar_t *cvar_cor_volt1;
CORE2_EXTERN core2_shell_cvar_t *cvar_cor_volt2;
CORE2_EXTERN core2_shell_cvar_t *cvar_cor_current;
CORE2_EXTERN core2_shell_cvar_t *cvar_cor_temp;
CORE2_EXTERN core2_shell_cvar_t *cvar_cor_press;
CORE2_EXTERN core2_shell_cvar_t *cvar_buzzer_enabled;
CORE2_EXTERN core2_shell_cvar_t *cvar_solenoid_trigger_time;
CORE2_EXTERN core2_shell_cvar_t *cvar_firmware_info_url;
CORE2_EXTERN core2_shell_cvar_t *cvar_firmware_bin_url;


void variables_init();

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
}
#endif