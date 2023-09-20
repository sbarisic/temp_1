#include <core2.h>
#include <math.h>

#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#include <FishGL.h>
#include <FishGLShader.h>

#define LILYGO_KB_SLAVE_ADDRESS 0x55

#define BOARD_POWERON 10
#define BOARD_I2C_SDA 18
#define BOARD_I2C_SCL 8

#define tskGRAPHICS_PRIORITY 10

#define WIDTH 320
#define HEIGHT 240

Arduino_DataBus *bus = new Arduino_ESP32SPI(BOARD_TFT_DC, BOARD_TFT_CS, BOARD_SPI_SCK, BOARD_SPI_MOSI, BOARD_SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7789(bus, GFX_NOT_DEFINED, 1, false);
Arduino_Canvas *canvasGfx = new Arduino_Canvas(WIDTH, HEIGHT, gfx);

#define rnd_num(min, max) ((rand() % (max - min)) + min)

int Width;
int Height;

FglBuffer ColorBuffer;
FglBuffer TestTex;
// FglBuffer Test2Tex;

FglColor Red;
FglColor Green;
FglColor Blue;
FglColor White;

#define TRI_COUNT 1
FglTriangle3 Tri[TRI_COUNT];
FglTriangle2 UV[TRI_COUNT];

typedef struct
{
    uint16_t x;
    uint16_t y;
} vec2_;

typedef struct
{
    vec2_ a;
    vec2_ b;
} line2;

vec2_ make_vec(float x, float y)
{
    uint16_t xx = (uint16_t)(50 + (uint16_t)(x * 20));
    uint16_t yy = (uint16_t)(240 - (20 + (uint16_t)(y * 20)));

    vec2_ v = {xx, yy};
    return v;
}

line2 make_line(float A, float B, float C, float D)
{
    line2 l = {};

    l.a = make_vec(A, B);
    l.b = make_vec(C, D);

    return l;
}

void InitShit()
{
    White = (FglColor){255, 255, 255};
    Red = (FglColor){255, 0, 0};
    Green = (FglColor){0, 255, 0};
    Blue = (FglColor){0, 0, 255};
}

bool VertexShader(FglState *State, vec3 Vert)
{
    FglColor Clr = Red;

    if (State->VertNum == 1)
        Clr = Green;
    else if (State->VertNum == 2)
        Clr = Blue;

    FglVarying *V = fglShaderGetVarying(1);
    V->Vec3[XElement] = Clr.R;
    V->Vec3[YElement] = Clr.G;
    V->Vec3[ZElement] = Clr.B;

    return FGL_KEEP;
}

bool FragmentShader(FglState *State, vec2 UV, FglColor *OutColor)
{
    vec2 newUV = {UV[0], UV[1]};
    // newUV[YElement] = 1 - newUV[YElement];

    FglColor C = fglShaderSampleTextureUV(&State->Textures[0], newUV);

    vec3 C2;
    glm_vec_copy(fglShaderGetVarying(1)->Vec3, C2);

    C.R = (int)(C.R * (C2[XElement] / 255));
    C.G = (int)(C.G * (C2[YElement] / 255));
    C.B = (int)(C.B * (C2[ZElement] / 255));

    *OutColor = C;

    return FGL_KEEP;
}

void fglDebugInit(int W, int H, int BPP)
{
    int32_t Len = 0;

    Width = W;
    Height = H;
    InitShit();

    ColorBuffer = fglCreateBuffer(malloc(W * H * sizeof(FglColor)), W, H);

    FglColor *Buff1 = (FglColor *)malloc(sizeof(FglColor) * 4);
    // FglColor *Buff2 = (FglColor *)malloc(sizeof(FglColor) * 4);

    Buff1[0] = Red;
    Buff1[1] = Green;
    Buff1[2] = Blue;
    Buff1[3] = White;

    TestTex = fglCreateBuffer(Buff1, 2, 2);
    // Test2Tex = fglCreateBuffer(Buff2, 2, 2);

    // TestTex = fglCreateBufferFromPng(LoadFile("test.png", &Len), Len);
    // Test2Tex = fglCreateBufferFromPng(LoadFile("test2.png", &Len), Len);

    fglBindTexture(&TestTex, 0);
    // fglBindTexture(&Test2Tex, 1);

    fglBindShader((void *)&VertexShader, FglShaderType_Vertex);
    fglBindShader((void *)&FragmentShader, FglShaderType_Fragment);

    float X = 50;
    float Y = 25;
    float SX = 200;
    float SY = 200;

    // Tri[0] = (FglTriangle3){{X, Y, 0}, {X + SX, Y, 0}, {X, Y + SY, 0}};
    // UV[0] = (FglTriangle2){{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}};

    // Tri[1] = (FglTriangle3){{X + SX, Y, 0}, {X + SX, Y + SY, 0}, {X, Y + SY, 0}};
    // UV[1] = (FglTriangle2){{1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    Tri[0] = (FglTriangle3){{X, Y, 0}, {X + SX, Y, 0}, {X, Y + SY, 0}};
    UV[0] = (FglTriangle2){{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}};
}

void fglDebugLoop()
{
    fglClearBuffer(&ColorBuffer, (FglColor){82, 18, 13});

    for (size_t i = 0; i < TRI_COUNT; i++)
        fglRenderTriangle3(&ColorBuffer, &Tri[i], &UV[i]);

    gfx->draw24bitRGBBitmap(0, 0, (uint8_t *)ColorBuffer.Memory, ColorBuffer.Width, ColorBuffer.Height);

    // fglDisplayToFramebuffer(&ColorBuffer);
}

void core2_gpu_main(void *args)
{
    dprintf("core2_gpu_main BEGIN\n");
    canvasGfx->begin(80000000);
    gfx->invertDisplay(true);

    fglDebugInit(WIDTH, HEIGHT, 24);

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

    ulong ms = millis();
    ulong ms_now = 0;
    ulong frame_time = 0;
    ulong frame_counter = 0;

    float xoffset = 0;

    for (;;)
    {
        /*gfx->fillScreen(RED);

        for (int i = 0; i < (sizeof(lines) / sizeof(*lines)); i++)
        {
            gfx->drawLine(lines[i].a.x + xoffset, lines[i].a.y, lines[i].b.x + xoffset, lines[i].b.y, WHITE);
        }

        xoffset += 1;

        if (xoffset > 100)
            xoffset = 0;*/

        // gfx->drawCircle(320 / 2, 240 / 2, 50, WHITE);

        fglDebugLoop();
        // gfx->flush();

        ms_now = millis();
        frame_time = ms_now - ms;
        ms = ms_now;

        if ((frame_counter++) % 10 == 0)
        {
            dprintf("Frame time: %lu ms - %.2f FPS\n", frame_time, (1.0f / (frame_time / 1000.0f)));
        }

        vTaskDelay(pdMS_TO_TICKS(2));
        // vTaskDelay(1);
    }
}

void core2_main_tdeck()
{
    pinMode(BOARD_POWERON, OUTPUT);
    digitalWrite(BOARD_POWERON, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));

    TaskHandle_t gfxTask;
    xTaskCreatePinnedToCore(core2_gpu_main, "core2_gpu_main", 4096 * 2, NULL, tskGRAPHICS_PRIORITY, &gfxTask, 0);

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