#include <core2.h>
#include <esp_http_server.h>

core2_shell_cvar_t cvar_testString;
core2_shell_cvar_t cvar_testString2;
core2_shell_cvar_t cvar_testInt;
core2_shell_cvar_t cvar_getvar_count;

float Volts[200];

bool send_data_to_server(char *json_buffer, size_t json_len)
{
    dprintf("Sending data!\n");
    if (!core2_web_json_post("https://demo.sbarisic.com/deviceaccess", json_buffer, strlen(json_buffer)))
    {
        return false;
    }

    return true;
}

void interrupt_read_voltage()
{
    bool adc_lock = core2_adc_lock();
    dprintf("Citam napone ... ");

    for (int i = 0; i < 200; i++)
    {
        core2_adc_read_ex(&(Volts[i]), NULL, CORE2_ADC_CH2, false);
        Volts[i] = Volts[i] * 9.215 / 1000;

        // WORKING!
        // float V1, V2;
        // core2_adc_read(&V1, &V2);
        // Volts[i] = V2;

        vTaskDelay(pdMS_TO_TICKS(15));
    }

    dprintf("OK\n");

    core2_json_t *json = core2_json_create();

    core2_json_add_field_string(json, "APIKey", "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw="); // TODO
    core2_json_add_field_int(json, "Action", 1);
    core2_json_add_field(json, "Volts", &Volts, (sizeof(Volts) / sizeof(*Volts)), CORE2_JSON_FLOAT_ARRAY_DEC2);

    char time_now[32] = {0};
    core2_clock_time_now(time_now);
    core2_json_add_field_string(json, "LocalTime", time_now);

    char *json_buffer;
    size_t json_len;
    core2_json_serialize(json, &json_buffer, &json_len);
    core2_json_delete(json);

    printf("======= core2_json_test =======\n");
    printf("%s\n", json_buffer);
    printf("===============================\n");

    if (!core2_file_write_timesuffix("/sd/processing/volts_%s.json", json_buffer, json_len))
    {
        // Saving to file failed, send now?
        send_data_to_server(json_buffer, json_len);
    }

    core2_free(json_buffer);
    core2_adc_unlock(adc_lock);

    /*for (int i = 0; i < 200; i++)
    {
        dprintf("%.2f  ", Volts[i]);
    }*/
}

void core2_shellcmd_get_variables(core2_shell_func_params_t *params, int argc, char **argv)
{
    core2_json_t *json = core2_json_create();
    core2_json_begin_field(json, "vars", CORE2_JSON_BEGIN_ARRAY);

    size_t count = core2_shell_cvar_count();
    for (size_t i = 0; i < count; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);

        core2_json_add(json, CORE2_JSON_BEGIN_OBJECT);
        core2_json_add_field_string(json, "variable_name", cvar->name);

        switch (cvar->var_type)
        {
        case CORE2_CVAR_STRING:
            core2_json_add_field_string(json, "value", (const char *)cvar->var_ptr);
            break;

        case CORE2_CVAR_INT32:
            core2_json_add_field_int(json, "value", (int32_t)cvar->var_ptr);
            break;

        default:
            // TODO:
            core2_json_add_field_string(json, "value", "TODO: NOT IMPLEMENTED");
            break;
        }

        core2_json_add(json, CORE2_JSON_END_OBJECT);
    }

    core2_json_add(json, CORE2_JSON_END_ARRAY);

    char *json_buffer;
    size_t json_len;
    core2_json_serialize(json, &json_buffer, &json_len);
    core2_json_delete(json);

    httpd_resp_set_type((httpd_req_t *)params->ud1, "application/json");
    params->print(params, json_buffer);

    core2_free(json_buffer);

    int cnt = (int)cvar_getvar_count.var_ptr;
    cnt++;
    cvar_getvar_count.var_ptr = (void *)cnt;
}

void core2_print(void *self, const char *str)
{
    dprintf("%s", str);
}

void core2_main()
{
    printf("Hello World!\n");

    /*core2_array_run_tests();

    printf("Done!\n");
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }*/

    // TODO: Load varijabli iz flash memorije ili sa SD kartice
    core2_shell_cvar_register(&cvar_testString, "testString", (void *)"Test string value", CORE2_CVAR_STRING);
    core2_shell_cvar_register(&cvar_testInt, "testInt", 0, CORE2_CVAR_INT32);
    core2_shell_cvar_register(&cvar_getvar_count, "getvar_count", 0, CORE2_CVAR_INT32);
    core2_shell_load_cvars();

    core2_shell_register("get_variables", core2_shellcmd_get_variables);
    // core2_shell_save_cvars();

    core2_shell_func_params_t *params = (core2_shell_func_params_t *)core2_malloc(sizeof(core2_shell_func_params_t));
    params->print = core2_print;

    // core2_shell_invoke("set testString \"Hello CVar World!\"", &params);
    core2_shell_invoke("help", params);

    // TODO: Make handlebars local
    core2_http_start();

    // core2_wifi_ap_start();

    core2_wifi_try_connect("Tst", "123456789");
    // core2_wifi_try_connect("Serengeti", "srgt#2018");

    // core2_wifi_ap_start();

    // core2_gpio_set_interrupt0();

    for (;;)
    {
        if (core2_gpio_get_interrupt0())
        {
            core2_gpio_enable_interrupt0(false);

            dprintf("[INT 0]!\n");
            interrupt_read_voltage();

            core2_gpio_clear_interrupt0();
            core2_gpio_enable_interrupt0(true);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
