#include <core2.h>

float Volts[200];
bool NaponiReady = false;

void interrupt_read_voltage()
{
    bool adc_lock = core2_adc_lock();
    dprintf("Citam napone ... ");

    for (int i = 0; i < 200; i++)
    {
        // core2_adc_read_ex(Volts + i, NULL, CORE2_ADC_CH2, false);

        float V1, V2;
        core2_adc_read(&V1, &V2);
        Volts[i] = V2;

        vTaskDelay(pdMS_TO_TICKS(15));
    }

    dprintf("OK\n");
    core2_adc_unlock(adc_lock);

    NaponiReady = true;

    /*for (int i = 0; i < 200; i++)
    {
        dprintf("%.2f  ", Volts[i]);
    }*/
}

void send_data_to_server()
{
    dprintf("Sending data!\n");

    char *json_buffer;
    size_t json_len;

    core2_json_begin();

    const char *field1 = "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=";
    core2_json_add_field("APIKey", &field1, 0, CORE2_JSON_STRING);

    int field2 = 1;
    core2_json_add_field("Action", &field2, 0, CORE2_JSON_INT);

    core2_json_add_field("Test", &Volts, 200, CORE2_JSON_FLOAT_ARRAY);

    core2_json_serialize(&json_buffer, &json_len);

    printf("======= core2_json_test =======\n");
    printf("%s\n", json_buffer);
    printf("===============================\n");

    // Save to file
    char cur_time[21];
    core2_clock_time_now(cur_time);
    char filename[30];
    core2_clock_time_fmt(filename, sizeof(filename), "/sd/read_json_%d%m%Y_%H%M%S.txt");
    core2_file_write(filename, json_buffer, strlen(json_buffer));

    core2_web_json_post("https://demo.sbarisic.com/deviceaccess", json_buffer, strlen(json_buffer));

    core2_json_end(&json_buffer, &json_len);
}

void core2_main()
{
    // const char *JSON_txt = "{ \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"Napon1\": 16.789 }";
    // size_t JSON_txt_len = strlen(JSON_txt);

    printf("Hello World!\n");
    // core2_wifi_ap_start();

    float Volts[] = {0.0f, 0.0f};
    float Factors[] = {4.795f / 1000, 4.795f / 1000};
    float V1, V2;

    for (;;)
    {
        /*// core2_adc_read_ex(Volts, NULL, (core2_adc_channel_t)(CORE2_ADC_CH1 | CORE2_ADC_CH2), false);

        core2_adc_read(&V1, &V2);

        // V1 = Volts[0];
        // V2 = Volts[1];
        dprintf("V1 = %f, V2 = %f\n", V1, V2);

        // send_data_to_server();*/

        if (core2_gpio_get_interrupt0())
        {
            dprintf("Interupt!\n");
            interrupt_read_voltage();

            core2_gpio_clear_interrupt0();
        }

        if (NaponiReady)
        {
            NaponiReady = false;
            send_data_to_server();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    core2_shell_register("interrupt_read_voltage", interrupt_read_voltage);
    core2_shell_register("send", send_data_to_server);
}
