#include <core2.h>

float Volts[200];
bool NaponiReady = false;

void interrupt_read_voltage()
{
    bool adc_lock = core2_adc_lock();
    dprintf("Citam napone ... ");

    for (int i = 0; i < 200; i++)
    {
        core2_adc_read_ex(Volts + i, NULL, CORE2_ADC_CH2, false);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    dprintf("OK\n");
    core2_adc_unlock(adc_lock);
}

void send_data_to_server()
{
    const char *json_txt = "{ \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"Napon1\": 16.789 }";
    core2_web_json_post("https://141.147.10.10/deviceaccess", json_txt, strlen(json_txt));
}

void core2_main()
{
    // const char *JSON_txt = "{ \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"Napon1\": 16.789 }";
    // size_t JSON_txt_len = strlen(JSON_txt);

    printf("Hello World!\n");
    // core2_wifi_ap_start();

    float Volts[] = {0.0f, 0.0f};
    float V1, V2;
    float Factors[] = {4.795f / 1000, 1.0f};

    for (;;)
    {
        // core2_adc_read_ex(Volts, NULL, (core2_adc_channel_t)(CORE2_ADC_CH1 | CORE2_ADC_CH2), false);

        core2_adc_read(&V1, &V2);
        dprintf("V1 = %f, V2 = %f\n", V1, V2);

        // send_data_to_server();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    core2_shell_register("interrupt_read_voltage", interrupt_read_voltage);
    core2_shell_register("send", send_data_to_server);
}
