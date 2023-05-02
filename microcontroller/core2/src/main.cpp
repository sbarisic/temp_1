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

  lock = core2_lock_create();
  
  core2_oled_print("Hello 1");
  core2_oled_print("Hello 2");
  core2_oled_print("Hello 3");
  core2_oled_print("Hello 4");
  core2_oled_print("Hello 5");
  core2_oled_print("Hello 6");
  core2_oled_print("Hello 7");
  core2_oled_print("Hello 8");
  core2_oled_print("Hello 9");

  xTaskCreate(test, "test", 4096, NULL, 1, NULL);
  xTaskCreate(test2, "test2", 4096, NULL, 1, NULL);
}

void loop()
{
}