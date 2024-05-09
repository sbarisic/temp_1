#include <core2.h>

#ifdef CORE2_CAN
#include <ESP32CAN.h>
#include <WS2812FX.h>

#define LED_PIN WS2812_PIN // digital pin used to drive the LED strip
#define LED_COUNT 1        // number of LEDs on the strip

#define RGB(R, G, B) ((R << 16) | (G << 8) | B)

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);
volatile bool color_ready;

void core2_ws2812fx_service(void *args)
{
    ws2812fx.init();
    ws2812fx.setBrightness(50);

    int val = ((0x00 << 16) | (0xFF << 8) | 0x0);

    uint32_t colors[] = {RGB(0x00, 0xFF, 0x00)};
    ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, colors, 0, NO_OPTIONS);
    ws2812fx.start();
    color_ready = true;

    for (;;)
    {
        ws2812fx.service();
        vPortYield();
    }
}

// CAN stuff =============================================================

CAN_device_t CAN_cfg;             // CAN Config
unsigned long previousMillis = 0; // will store last time a CAN Message was send
const int interval = 1000;        // interval at which send CAN Messages (milliseconds)
const int rx_queue_size = 10;     // Receive Queue size

void core2_can_init()
{
    CAN_cfg.speed = CAN_SPEED_500KBPS;
    CAN_cfg.tx_pin_id = CAN_TX_PIN;
    CAN_cfg.rx_pin_id = CAN_RX_PIN;
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    // Init CAN Module
    ESP32Can.CANInit();
}

void core2_can_send(uint32_t msg_id, uint8_t dlc, uint8_t *arr)
{
    CAN_frame_t tx_frame;
    tx_frame.FIR.B.FF = CAN_frame_std;
    tx_frame.MsgID = msg_id;
    tx_frame.FIR.B.DLC = dlc;
    memcpy(tx_frame.data.u8, arr, dlc);

    ESP32Can.CANWriteFrame(&tx_frame);
}

void core2_can_task(void *args)
{
    core2_gpio_set_output(CAN_SE_PIN);
    gpio_set_level(CAN_SE_PIN, LOW);

    for (;;)
    {
        CAN_frame_t rx_frame;

        // Receive next CAN frame from queue
        if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
        {

            if (rx_frame.FIR.B.FF == CAN_frame_std)
            {
                dprintf("New standard frame");
            }
            else
            {
                dprintf("New extended frame");
            }

            if (rx_frame.FIR.B.RTR == CAN_RTR)
            {
                dprintf(" RTR from 0x%08X, DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            }
            else
            {
                dprintf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
                for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
                {
                    dprintf("0x%02X ", rx_frame.data.u8[i]);
                }
                dprintf("\n");
            }
        }

        vPortYield();
    }
}

void core2_can_main()
{
    dprintf("Hello CAN World!\n");

    core2_gpio_set_output(PIN_5V_EN);
    gpio_set_level(PIN_5V_EN, HIGH);

    core2_wifi_add_network("Barisic", "123456789");
    core2_wifi_add_network("Serengeti", "srgt#2018");
    core2_wifi_add_network("Tst", "123456789");

    color_ready = false;
    xTaskCreate(core2_ws2812fx_service, "ws2812fx.service", 1024 * 16, NULL, 10, NULL);

    for (;;)
    {
        if (color_ready)
        {
            color_ready = false;

            for (size_t i = 0; i < 20; i++)
            {
                ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(0xFF, 0x00, 0x00)), 0, NO_OPTIONS);
                core2_sleep(200);

                ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(0x00, 0x00, 0xFF)), 0, NO_OPTIONS);
                core2_sleep(200);
            }
        }

        ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(168, 50, 157)), 0, NO_OPTIONS);
        core2_sleep(1000);

        ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(237, 166, 33)), 0, NO_OPTIONS);
        core2_sleep(1000);
    }
}

#endif