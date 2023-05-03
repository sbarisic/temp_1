#include <Arduino.h>

#include <core2.h>

static SemaphoreHandle_t lock;

void test(void *params)
{
  while (true)
  {
    core2_lock_begin(lock);
    core2_oled_print("The quick brown fox");
    core2_lock_end(lock);

    vTaskDelay(pdMS_TO_TICKS(800));
  }
}

void test2(void *params)
{
  while (true)
  {
    core2_lock_begin(lock);
    core2_oled_print("jumps over the lazy dog");
    core2_lock_end(lock);

    vTaskDelay(pdMS_TO_TICKS(900));
  }
}

void setup()
{
  core2_init();
  core2_oled_init();
  core2_wifi_init();

  core2_print_status();

  vTaskDelay(pdMS_TO_TICKS(5000));

  core2_clock_time_now();

  /*lock = core2_lock_create();

  xTaskCreate(test, "test", 4096, NULL, 1, NULL);
  xTaskCreate(test2, "test2", 4096, NULL, 1, NULL);*/

  // Stop arduino task, job done
  vTaskDelete(NULL);
}

void loop()
{
}