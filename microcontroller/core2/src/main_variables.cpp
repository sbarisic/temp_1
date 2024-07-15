#include <core2.h>
#include <core2_variables.h>

void variables_init()
{
    // Naziv vozila
    cvar_vehicle_name = core2_shell_cvar_register("vehicle_name", NULL, CORE2_CVAR_STRING);
    cvar_vehicle_name->var_perm = CORE2_CVAR_PERM_USER;

    cvar_vehicle_number = core2_shell_cvar_register("vehicle_number", NULL, CORE2_CVAR_INT32);
    cvar_vehicle_number->var_perm = CORE2_CVAR_PERM_USER;

    cvar_network_ssid = core2_shell_cvar_register("wifi_ssid", (void *)"Test", CORE2_CVAR_STRING);
    cvar_network_ssid->var_perm = CORE2_CVAR_PERM_USER;

    cvar_network_password = core2_shell_cvar_register("wifi_pass", (void *)"12345678910", CORE2_CVAR_STRING);
    cvar_network_password->var_perm = CORE2_CVAR_PERM_USER;

    cvar_ap_ssid = core2_shell_cvar_register("wifi_ap_ssid", (void *)"core2_wifi_devtest", CORE2_CVAR_STRING);
    cvar_ap_password = core2_shell_cvar_register("wifi_ap_pass", (void *)"core21234", CORE2_CVAR_STRING);
    cvar_host = core2_shell_cvar_register();
    cvar_port = core2_shell_cvar_register();
    cvar_api_key = core2_shell_cvar_register();
    cvar_cor_volt1 = core2_shell_cvar_register();
    cvar_cor_volt2 = core2_shell_cvar_register();
    cvar_cor_current = core2_shell_cvar_register();
    cvar_cor_temp = core2_shell_cvar_register();
    cvar_cor_press = core2_shell_cvar_register();
    cvar_buzzer_enabled = core2_shell_cvar_register();
    cvar_solenoid_trigger_time = core2_shell_cvar_register();
    core2_shell_load_cvars();
}
