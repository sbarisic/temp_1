#include <Arduino.h>
#include <core2.h>

void main_logic(void *params)
{
    dprintf("main_logic()\n");
    int counter = 0;

    while (true)
    {
        if (core2_gpio_get_interrupt0())
        {
            counter++;

            for (size_t i = 0; i < 10; i++)
            {
                printf("NOW: %lld\n", esp_timer_get_time());
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            char buffer[32];
            sprintf(buffer, "Button %d", counter);
            core2_oled_print(buffer);
        }
    }

    vTaskDelete(NULL);
}

void setup()
{
    core2_init();
    core2_print_status();

    core2_gpio_init();
    core2_oled_init();
    core2_wifi_init();
    core2_clock_init();

    core2_wifi_yield_until_connected();
    dprintf("init() done\n");

    char cur_time[21];
    core2_clock_time_now(cur_time);
    dprintf("Current date time: %s\n", cur_time);

    xTaskCreate(main_logic, "main_logic", 1024 * 16, NULL, 1, NULL);

    // Stop arduino task, job done
    vTaskDelete(NULL);
}

void loop()
{
}
