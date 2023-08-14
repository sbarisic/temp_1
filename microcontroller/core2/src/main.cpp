#include <Arduino.h>
#include <core2.h>

float scope_volts[200];
bool scope_in_progress;

void read_battery_scope()
{
    memset(scope_volts, 0, sizeof(scope_volts));

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

        scope_volts[i] = minnapon;
    }
}

void main_logic(void *params)
{
    dprintf("main_logic()\n");
    int counter = 0;
    scope_in_progress = false;

    char temp_buffer[64];

    while (true)
    {
        if (core2_gpio_get_interrupt0())
        {
            if (scope_in_progress)
                continue;

            scope_in_progress = true;

            // TODO: Maybe slow?
            if (true)
            {
                counter++;
                sprintf(temp_buffer, "Button %d", counter);
                core2_oled_print(temp_buffer);
            }

            // Read scope
            dprintf("Reading scope ...\n");
            read_battery_scope();
            dprintf("Reading scope ... OK\n");

            // Drop file to SD card
            dprintf("Saving to file\n");
            core2_clock_time_fmt(temp_buffer, sizeof(temp_buffer), "/sd/processing/scope_%d%m%Y_%H%M%S.txt");

            FILE *f = core2_file_open(temp_buffer);

            for (size_t i = 0; i < (sizeof(scope_volts) / sizeof(*scope_volts)); i++)
            {
                fprintf(f, "%.2f\n", scope_volts[i]);
            }

            core2_file_close(f);

            core2_gpio_clear_interrupt0();
            scope_in_progress = false;

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

void network_logic(void *params)
{
    dprintf("network_logic()\n");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000 * 10));

        // Busy, skip
        if (scope_in_progress)
            continue;

        if (core2_wifi_isconnected())
        {
            core2_file_list("/sd/processing", [](const char *full_name, const char *file_name)
                            { core2_file_move(full_name, "/sd/logs/"); });
        }
    }

    vTaskDelete(NULL);
}

void analog_voltage_test()
{
    dprintf("======================== analog_voltage_test()\n");

    float a, b;
    core2_adc_read(&a, &b);

    dprintf("V1 = %f, V2 = %f\n", a, b);
}

void append_file_test()
{
    dprintf("======================== append_file_test()\n");
    core2_file_append("/sd/test.txt", "Hello!\n", 7);
}

void run_tests()
{
    printf("Starting tests\n");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));
        analog_voltage_test();

        vTaskDelay(pdMS_TO_TICKS(3000));
        append_file_test();
    }
}

void setup()
{
    core2_init();
    core2_print_status();

    core2_spi_init();
    sdmmc_host_t sdcard_host;
    if (core2_spi_create(&sdcard_host, SDCARD_PIN_MOSI, SDCARD_PIN_MISO, SDCARD_PIN_CLK))
    {
        core2_filesystem_init(&sdcard_host, SDCARD_PIN_CS);
    }

    core2_mcp320x_init();
    run_tests();

    core2_flash_init();
    core2_gpio_init();
    core2_oled_init();
    core2_wifi_init();
    core2_clock_init();
    core2_json_init();

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
    xTaskCreate(network_logic, "network_logic", 1024 * 16, NULL, 1, NULL);

    // Stop arduino task, job done
    vTaskDelete(NULL);
}

void loop()
{
}
