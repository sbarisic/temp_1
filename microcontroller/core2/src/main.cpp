#include <Arduino.h>
#include <core2.h>

void setup()
{
  core2_oled_init();

  core2_oled_print("Hello 1", true);
  core2_oled_print("Hello 2", true);
  core2_oled_print("Hello 3", true);
  core2_oled_print("Hello 4", true);
  core2_oled_print("Hello 5", true);
  core2_oled_print("Hello 6", true);
  core2_oled_print("Hello 7", true);
  core2_oled_print("Hello 8", true);
  core2_oled_print("Hello 9", true);
}

void loop()
{
}