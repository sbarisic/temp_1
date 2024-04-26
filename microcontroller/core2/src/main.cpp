#include <core2.h>

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

void core2_shellcmd_get_variables(core2_shell_func_params_t *params)
{
    params->print(
        params,
        "{ \"vars\": [{ \"variable_name\": \"Var 1\" }, { \"variable_name\": \"Var 2\", \"value\": \"Some Value\"}] }");
}

void core2_main()
{
    printf("Hello World!\n");
    core2_shell_register("get_variables", core2_shellcmd_get_variables);

    core2_http_start();
    core2_wifi_try_connect("Barisic", "123456789");
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
