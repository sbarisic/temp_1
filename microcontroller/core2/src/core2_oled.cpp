#include <core2.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <freertos/FreeRTOS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


static const uint8_t disp_mem[(SCREEN_WIDTH * SCREEN_HEIGHT) / 8] = {0};


int c2_disp_mem_get(int X, int Y)
{
    int Bit = Y * SCREEN_WIDTH + X;
    return 0;
}

void c2_disp_mem_set(int X, int Y, int V)
{
    
}

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 *display;

void c2_display()
{
    display->drawBitmap(0, 0, disp_mem, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
    display->display();
}

void core2_oled_init()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

    // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
    Wire.begin(5, 4);

    if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false))
    {
        // Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }

    //display->clearDisplay();

    //display->setCursor(0, 0);
    //display->setTextSize(1);
    //display->setTextColor(WHITE);
}

void core2_oled_print(const char *txt, bool newline)
{
}