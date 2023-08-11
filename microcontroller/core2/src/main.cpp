#include <Arduino.h>
#include <core2.h>

float naponi[200];
bool naponi_available;

void read_battery_scope()
{
    for (int i = 0; i < 200; i++)
    {
        naponi[i] = 0;
    }

    

    for (int i = 0; i < 200; i++)
    {
        float minnapon = 35.0;
        for (int j = 0; j < 20; j++)
        {
            // uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);
            // uint16_t val1 = adc1.toAnalog(raw1);
            uint16_t raw1 = 10;
            uint16_t val1 = 10;

            float naponj = val1 * 9.215 / 1000;

            vTaskDelay(pdMS_TO_TICKS(1));

            if (naponj < minnapon)
                minnapon = naponj;
        }

        naponi[i] = minnapon;
    }
}

void main_logic(void *params)
{
    dprintf("main_logic()\n");
    int counter = 0;

    while (true)
    {
        if (core2_gpio_get_interrupt0())
        {
            if (naponi_available)
                continue;
            naponi_available = true;

            counter++;
            char buffer[64];
            sprintf(buffer, "Button %d", counter);
            core2_oled_print(buffer);

            // Read scope
            dprintf("Reading scope ... ");
            read_battery_scope();
            dprintf("OK\n");

            // Drop file to SD card
            dprintf("Saving to file\n");
            char filename[40];
            core2_clock_time_fmt(filename, sizeof(filename), "/sd/logs/scope_%d%m%Y_%H%M%S.txt");

            FILE *f = core2_file_open(filename);

            for (size_t i = 0; i < (sizeof(naponi) / sizeof(*naponi)); i++)
            {
                fprintf(f, "%.2f\n", naponi[i]);
            }

            core2_file_close(f);

            /*for (size_t i = 0; i < 7; i++)
            {
                int64_t uS = esp_timer_get_time();
                sprintf(buffer, "NOW: %lld uS, %lld ms, %f s\n", uS, uS / 1000, ((float)uS) / 1000000.0f);

                printf(buffer);
                // core2_oled_print(buffer);

                vTaskDelay(pdMS_TO_TICKS(10));
            }*/
        }
    }

    vTaskDelete(NULL);
}

void setup()
{
    naponi_available = false;

    core2_init();
    core2_print_status();

    core2_flash_init();
    core2_filesystem_init();
    core2_gpio_init();
    core2_mcp320x_init();
    core2_oled_init();
    core2_wifi_init();
    core2_clock_init();

    core2_wifi_yield_until_connected();
    dprintf("init() done\n");

    char cur_time[21];
    core2_clock_time_now(cur_time);
    dprintf("Current date time: %s\n", cur_time);

    char filename[30];
    core2_clock_time_fmt(filename, sizeof(filename), "/sd/boot_%d%m%Y_%H%M%S.txt");

    const char *Text = "Hello ESP32 World!\n";
    core2_file_write(filename, Text, strlen(Text));

    xTaskCreate(main_logic, "main_logic", 1024 * 16, NULL, 1, NULL);

    // Stop arduino task, job done
    vTaskDelete(NULL);
}

void loop()
{
}
