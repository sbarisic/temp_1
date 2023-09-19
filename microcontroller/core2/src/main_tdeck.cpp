#include <core2.h>


#define LILYGO_KB_SLAVE_ADDRESS 0x55

#define BOARD_POWERON 10
#define BOARD_I2C_SDA 18
#define BOARD_I2C_SCL 8

void core2_main_tdeck()
{
    pinMode(BOARD_POWERON, OUTPUT);
    digitalWrite(BOARD_POWERON, HIGH);

    pinMode(BOARD_TFT_BACKLIGHT, OUTPUT);
    digitalWrite(BOARD_TFT_BACKLIGHT, HIGH);

    dprintf("Hello World!\n");
    int counter = 0;

    for (;;)
    {
        dprintf("Hello World! %d\n", counter++);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}