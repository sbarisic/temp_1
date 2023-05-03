#include <Arduino.h>

#include <core2.h>

static SemaphoreHandle_t lock;

void main_logic(void *params)
{
    core2_wifi_yield_until_connected();

    core2_clock_time_now();

     vTaskDelete(NULL);
}

void setup()
{
    core2_init();
    core2_print_status();

    core2_oled_init();
    core2_wifi_init();

    xTaskCreate(main_logic, "main_logic", 1024 * 8, NULL, 1, NULL);

    // Stop arduino task, job done
    vTaskDelete(NULL);
}

void loop()
{
}