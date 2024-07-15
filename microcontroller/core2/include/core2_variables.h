#pragma once
#include <core2.h>

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CORE2_WINDOWS
#endif

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
extern "C"
{
#endif


// User variables
core2_shell_cvar_t *cvar_vehicle_name;
core2_shell_cvar_t *cvar_vehicle_number;
core2_shell_cvar_t *cvar_network_ssid;
core2_shell_cvar_t *cvar_network_password;


core2_shell_cvar_t *cvar_ap_ssid;
core2_shell_cvar_t *cvar_ap_password;
core2_shell_cvar_t *cvar_host;
core2_shell_cvar_t *cvar_port;
core2_shell_cvar_t *cvar_api_key;
core2_shell_cvar_t *cvar_cor_volt1;
core2_shell_cvar_t *cvar_cor_volt2;
core2_shell_cvar_t *cvar_cor_current;
core2_shell_cvar_t *cvar_cor_temp;
core2_shell_cvar_t *cvar_cor_press;
core2_shell_cvar_t *cvar_buzzer_enabled;
core2_shell_cvar_t *cvar_solenoid_trigger_time;


void variables_init();

#if defined(__cplusplus) && defined(CORE2_WINDOWS)
}
#endif