#include <core2.h>

#define CORE2_VARIABLES_IMPL
#include <core2_variables.h>

void variables_init()
{
    // Naziv vozila
    cvar_vehicle_name = core2_shell_cvar_register_string("vehicle_name", "vehicle_default");
    cvar_vehicle_name->var_perm = CORE2_CVAR_PERM_USER;

    cvar_vehicle_number = core2_shell_cvar_register_int32("vehicle_number", 0);
    cvar_vehicle_number->var_perm = CORE2_CVAR_PERM_USER;

    cvar_network_ssid = core2_shell_cvar_register_string("wifi_ssid", "Test");
    cvar_network_ssid->var_perm = CORE2_CVAR_PERM_USER;

    cvar_network_password = core2_shell_cvar_register_string("wifi_pass", "12345678910");
    cvar_network_password->var_perm = CORE2_CVAR_PERM_USER;

    cvar_ap_ssid = core2_shell_cvar_register_string("wifi_ap_ssid", "core2_wifi_devtest");
    cvar_ap_password = core2_shell_cvar_register_string("wifi_ap_pass", "core21234");

    cvar_host = core2_shell_cvar_register_string("api_host", "demo.sbarisic.com");
    cvar_port = core2_shell_cvar_register_int32("api_port", 443);
    cvar_api_key = core2_shell_cvar_register_string("api_key", "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=");

    cvar_cor_volt1 = core2_shell_cvar_register_float("cor_volt1", 1.0f);
    cvar_cor_volt2 = core2_shell_cvar_register_float("cor_volt2", 1.0f);
    cvar_cor_current = core2_shell_cvar_register_float("cor_current", 1.0f);
    cvar_cor_temp = core2_shell_cvar_register_float("cor_temp", 1.0f);
    cvar_cor_press = core2_shell_cvar_register_float("cor_press", 1.0f);

    cvar_buzzer_enabled = core2_shell_cvar_register_int32("hw_buzz_enabled", 1);
    cvar_solenoid_trigger_time = core2_shell_cvar_register_int32("hw_solenoid_trig_time", 300);

    core2_shell_load_cvars();
}
