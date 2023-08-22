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
}

void core2_main()
{
    const char *JSON_txt = "{ \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"Napon1\": 16.789 }";
    size_t JSON_txt_len = strlen(JSON_txt);

    printf("Hello World!\n");
}
