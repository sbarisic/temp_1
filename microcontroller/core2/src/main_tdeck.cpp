#include <core2.h>

#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#define LILYGO_KB_SLAVE_ADDRESS 0x55

#define BOARD_POWERON 10
#define BOARD_I2C_SDA 18
#define BOARD_I2C_SCL 8

Arduino_DataBus *bus = new Arduino_ESP32SPI(BOARD_TFT_DC, BOARD_TFT_CS, BOARD_SPI_SCK, BOARD_SPI_MOSI, BOARD_SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7789(bus, GFX_NOT_DEFINED, 1, false);
Arduino_Canvas *canvasGfx = new Arduino_Canvas(320, 240, gfx);

typedef struct
{
    uint16_t x;
    uint16_t y;
} vec2;

typedef struct
{
    vec2 a;
    vec2 b;
} line2;

vec2 make_vec(float x, float y)
{
    vec2 v = {50 + (uint16_t)(x * 20), 240 - (20 + (uint16_t)(y * 20))};
    return v;
}

line2 make_line(float A, float B, float C, float D)
{
    line2 l = {};

    l.a = make_vec(A, B);
    l.b = make_vec(C, D);

    return l;
}

void core2_gpu_thread(void *args)
{
    dprintf("core2_gpu_thread BEGIN\n");
    canvasGfx->begin(80000000);
    gfx->invertDisplay(true);

    ledcSetup(0, 1000, 8);
    ledcAttachPin(BOARD_TFT_BACKLIGHT, 0);
    ledcWrite(0, 180);

    line2 lines[] = {
        make_line(3, 2, 2.3825f, 2.20125f),
        make_line(2.3825f, 2.20125f, 2.0025f, 2.62125f),
        make_line(2.0025f, 2.62125f, 1.8825f, 3.20125f),
        make_line(1.8825f, 3.20125f, 2.0025f, 3.70125f),
        make_line(2.0025f, 3.70125f, 2.3625f, 4.06125f),
        make_line(2.3625f, 4.06125f, 2.8825f, 4.18125f),
        make_line(2.8825f, 4.18125f, 3.5025f, 4.04125f),
        make_line(3.5025f, 4.04125f, 3.7825f, 3.76125f),
        make_line(3.7825f, 3.76125f, 5.7425f, 6.42125f),
        make_line(5.7425f, 6.42125f, 5.4225f, 6.50125f),
        make_line(5.4225f, 6.50125f, 5.4225f, 6.90125f),
        make_line(5.4225f, 6.90125f, 5.7225f, 7.38125f),
        make_line(5.7225f, 7.38125f, 6.3225f, 7.76125f),
        make_line(6.3225f, 7.76125f, 6.8425f, 7.90125f),
        make_line(6.8425f, 7.90125f, 7.3625f, 7.90125f),
        make_line(7.3625f, 7.90125f, 7.8025f, 7.80125f),
        make_line(7.8025f, 7.80125f, 8.0625f, 7.50125f),
        make_line(8.0625f, 7.50125f, 8.1825f, 7.12125f),
        make_line(8.1825f, 7.12125f, 8.1825f, 6.66125f),
        make_line(8.1825f, 6.66125f, 8, 6),
        make_line(8, 6, 7.6825f, 5.54125f),
        make_line(7.6825f, 5.54125f, 7.2225f, 5.44125f),
        make_line(7.2225f, 5.44125f, 6.9625f, 5.74125f),
        make_line(6.9625f, 5.74125f, 5.1425f, 3.24125f),
        make_line(5.1425f, 3.24125f, 5.6025f, 3.32125f),
        make_line(5.6025f, 3.32125f, 6.0625f, 3.26125f),
        make_line(6.0625f, 3.26125f, 6.4825f, 2.86125f),
        make_line(6.4825f, 2.86125f, 6.5625f, 2.24125f),
        make_line(6.5625f, 2.24125f, 6.2025f, 1.70125f),
        make_line(6.2025f, 1.70125f, 5.5225f, 1.30125f),
        make_line(5.5225f, 1.30125f, 4.8625f, 1.20125f),
        make_line(4.8625f, 1.20125f, 4.2225f, 1.44125f),
        make_line(4.2225f, 1.44125f, 4.0f, 2.0f),
        make_line(7.5025f, 7.36125f, 7.8025f, 7.80125f)};

    for (;;)
    {
        gfx->fillScreen(RED);

        for (int i = 0; i < (sizeof(lines) / sizeof(*lines)); i++)
        {
            gfx->drawLine(lines[i].a.x, lines[i].a.y, lines[i].b.x, lines[i].b.y, WHITE);
        }

        //gfx->drawCircle(320 / 2, 240 / 2, 50, WHITE);
        gfx->flush();

        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

void core2_main_tdeck()
{
    pinMode(BOARD_POWERON, OUTPUT);
    digitalWrite(BOARD_POWERON, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));

    TaskHandle_t gfxTask;
    xTaskCreatePinnedToCore(core2_gpu_thread, "core2_gpu_thread", 4096 * 2, NULL, 10, &gfxTask, 0);

    // pinMode(BOARD_TFT_BACKLIGHT, OUTPUT);
    // digitalWrite(BOARD_TFT_BACKLIGHT, HIGH);

    dprintf("Hello World!\n");

    for (;;)
    {
        /*gfx->fillScreen(RED);
        vTaskDelay(pdMS_TO_TICKS(1000));

        gfx->fillScreen(GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));

        gfx->fillScreen(BLUE);*/
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}