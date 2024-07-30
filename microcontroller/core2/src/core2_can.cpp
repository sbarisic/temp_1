#include <core2.h>
#include <core2_ecumaster.h>
#include <core2_server_udp.h>
#include <core2_update.h>
#include <core2_variables.h>

#ifdef CORE2_CAN
#include <ESP32CAN.h>
#include <WS2812FX.h>

#define LED_PIN WS2812_PIN // digital pin used to drive the LED strip
#define LED_COUNT 1        // number of LEDs on the strip
#define RGB(R, G, B) ((R << 16) | (G << 8) | B)

typedef struct
{
    CAN_frame_t frame;
    int64_t next_send;
    int16_t send_interval;
} core2_tx_can_frame_t;

void variables_init();

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);
bool update_anim = false;
bool color_ready = false;

core2_tx_can_frame_t tx_frames[16];
int tx_frames_count = 0;

void core2_ws2812fx_service(void *args)
{
    dprintf("Beginning LED task\n");

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

CAN_device_t CAN_cfg;         // CAN Config
const int rx_queue_size = 10; // Receive Queue size

FILE *log_file;
emu_data_t emu_data;
int64_t emu_tstp[8];

int64_t timestamp_get(uint32_t can_id)
{
    return emu_tstp[can_id - EMU_BASE];
}

void timestamp_set(uint32_t can_id, int64_t val)
{
    emu_tstp[can_id - EMU_BASE] = val;
}

int64_t timestamp_get_last(uint32_t can_id)
{
    return core2_clock_bootms() - timestamp_get(can_id);
}

bool emu_online()
{
    int64_t ms = core2_clock_bootms();

    for (size_t i = 0; i < 8; i++)
    {
        if ((ms - timestamp_get(EMU_BASE + i)) < 100)
            return true;
    }

    return false;
}

bool emu_cel_on()
{
    const uint16_t EFLG_ERRORMASK = ERR_CLT | ERR_IAT | ERR_MAP | ERR_WBO | ERR_EGT1 | ERR_EGT2 | EGT_ALARM | KNOCKING;

    if (emu_data.cel & EFLG_ERRORMASK)
        return true;

    return false;
}

bool emu_flag(FLAGS1 flag)
{
    return (emu_data.flags1 & flag) != 0;
}

bool emu_errorflag(ERRORFLAG flag)
{
    return (emu_data.cel & flag) != 0;
}

void core2_can_log(emu_data_t *emu, FILE *f)
{
}

bool decode_emu_frame(CAN_frame_t *frame)
{
    uint8_t *data = frame->data.u8;
    uint32_t can_id = frame->MsgID;
    uint8_t can_dlc = frame->FIR.B.DLC;
    const uint32_t _EMUbase = EMU_BASE;

    if (!(can_id >= _EMUbase && can_id <= _EMUbase + 7))
        return false;

    timestamp_set(can_id, core2_clock_bootms());

    // Base:
    if (can_id == _EMUbase)
    {
        // 0-1 RPM in 16Bit unsigned
        emu_data.RPM = (data[1] << 8) + data[0];
        // 2 TPS in /2 %
        emu_data.TPS = data[2] * 0.5;
        // 3 IAT 8bit signed -40-127°C
        emu_data.IAT = int8_t(data[3]);
        // 4-5 MAP 16Bit 0-600kpa
        emu_data.MAP = (data[5] << 8) + data[4];
        // 6-7 INJPW 0-50 0.016129ms
        emu_data.pulseWidth = ((data[7] << 8) + data[6]) * 0.016129;
    }
    // Base +1:
    if (can_id == _EMUbase + 1)
    {
        // AIN in 16Bit unsigned  0.0048828125 V/bit
        emu_data.analogIn1 = ((data[1] << 8) + data[0]) * 0.0048828125;
        emu_data.analogIn2 = ((data[3] << 8) + data[2]) * 0.0048828125;
        emu_data.analogIn3 = ((data[5] << 8) + data[4]) * 0.0048828125;
        emu_data.analogIn4 = ((data[7] << 8) + data[6]) * 0.0048828125;
    }
    // Base +2:
    if (can_id == _EMUbase + 2)
    {
        // 0-1 VSPD in 16Bit unsigned 1 kmh/h / bit
        emu_data.vssSpeed = (data[1] << 8) + data[0];
        // 2 BARO 50-130 kPa
        emu_data.Baro = data[2];
        // 3 OILT 0-160°C
        emu_data.oilTemperature = data[3];
        // 4 OILP BAR 0.0625 bar/bit
        emu_data.oilPressure = data[4] * 0.0625;
        // 5 FUELP BAR 0.0625 bar/bit
        emu_data.fuelPressure = data[5] * 0.0625;
        // 6-7 CLT 16bit Signed -40-250 1 C/bit
        emu_data.CLT = int16_t(((data[7] << 8) + data[6]));
    }
    // Base +3:
    if (can_id == _EMUbase + 3)
    {
        // 0 IGNANG in 8Bit signed    -60 60  0.5deg/bit
        emu_data.IgnAngle = int8_t(data[0]) * 0.5;
        // 1 DWELL 0-10ms 0.05ms/bit
        emu_data.dwellTime = data[1] * 0.05;
        // 2 LAMBDA 8bit 0-2 0.0078125 L/bit
        emu_data.wboLambda = data[2] * 0.0078125;
        // 3 LAMBDACORR 75-125 0.5%
        emu_data.LambdaCorrection = data[3] * 0.5;
        // 4-5 EGT1 16bit °C
        emu_data.Egt1 = ((data[5] << 8) + data[4]);
        // 6-7 EGT2 16bit °C
        emu_data.Egt2 = ((data[7] << 8) + data[6]);
    }
    // Base +4:
    if (can_id == _EMUbase + 4)
    {
        // 0 GEAR
        emu_data.gear = data[0];
        // 1 ECUTEMP °C
        emu_data.emuTemp = data[1];
        // 2-3 BATT 16bit  0.027 V/bit
        emu_data.Batt = ((data[3] << 8) + data[2]) * 0.027;
        // 4-5 ERRFLAG 16bit
        emu_data.cel = ((data[5] << 8) + data[4]);
        // 6 FLAGS1 8bit
        emu_data.flags1 = data[6];
        // 7 ETHANOL %
        emu_data.flexFuelEthanolContent = data[7];
    }
    // Base +5:
    if (can_id == _EMUbase + 5)
    {
        // 0 DBW Pos 0.5%
        emu_data.DBWpos = data[0] * 0.5;
        // 1 DBW Target 0.5%
        emu_data.DBWtarget = data[1] * 0.5;
        // 2-3 TC DRPM RAW 16bit  1/bit
        emu_data.TCdrpmRaw = ((data[3] << 8) + data[2]);
        // 4-5 TC DRPM 16bit  1/bit
        emu_data.TCdrpm = ((data[5] << 8) + data[4]);
        // 6 TC Torque reduction %
        emu_data.TCtorqueReduction = data[6];
        // 7 Pit Limit Torque reduction %
        emu_data.PitLimitTorqueReduction = data[7];
    }
    // Base +6:
    if (can_id == _EMUbase + 6)
    {
        // AIN in 16Bit unsigned  0.0048828125 V/bit
        emu_data.analogIn5 = ((data[1] << 8) + data[0]) * 0.0048828125;
        emu_data.analogIn6 = ((data[3] << 8) + data[2]) * 0.0048828125;
        emu_data.outflags1 = data[4];
        emu_data.outflags2 = data[5];
        emu_data.outflags3 = data[6];
        emu_data.outflags4 = data[7];
    }
    // Base +7:
    if (can_id == _EMUbase + 7)
    {
        // 0-1 Boost target 16bit 0-600 kPa
        emu_data.boostTarget = ((data[1] << 8) + data[0]);
        // 2 PWM#1 DC 1%/bit
        emu_data.pwm1 = data[2];
        // 3 DSG mode 2=P 3=R 4=N 5=D 6=S 7=M 15=fault
        emu_data.DSGmode = data[3];
        // since version 143 this contains more data, check length:
        if (can_dlc == 8)
        {
            // 4 Lambda target 8bit 0.01%/bit
            emu_data.lambdaTarget = data[4] * 0.01;
            // 5 PWM#2 DC 1%/bit
            emu_data.pwm2 = data[5];
            // 6-7 Fuel used 16bit 0.01L/bit
            emu_data.fuel_used = ((data[7] << 8) + data[6]) * 0.01;
        }
    }

    core2_can_log(&emu_data, log_file);
    return true;
}

void core2_can_init()
{
    dprintf("core2_can_init()\n");

    CAN_cfg.speed = CAN_SPEED_500KBPS;
    CAN_cfg.tx_pin_id = CAN_TX_PIN;
    CAN_cfg.rx_pin_id = CAN_RX_PIN;
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    // Init CAN Module
    ESP32Can.CANInit();
}

void core2_can_send_frame(const CAN_frame_t *tx_frame)
{
    ESP32Can.CANWriteFrame(tx_frame);
}

void core2_can_send(uint32_t msg_id, uint8_t dlc, uint8_t *arr)
{
    CAN_frame_t tx_frame;
    tx_frame.FIR.B.FF = CAN_frame_std;
    tx_frame.MsgID = msg_id;
    tx_frame.FIR.B.DLC = dlc;
    memcpy(tx_frame.data.u8, arr, dlc);

    core2_can_send_frame(&tx_frame);
}

void core2_can_recv_task(void *args)
{
    core2_gpio_set_output(CAN_SE_PIN, CORE2_GPIO_MODE_NONE);
    gpio_set_level(CAN_SE_PIN, LOW);

    for (;;)
    {
        CAN_frame_t rx_frame;

        // Receive next CAN frame from queue
        if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
        {
            if (!decode_emu_frame(&rx_frame))
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
        }

        vPortYield();
    }

    vTaskDelete(NULL);
}

void core2_can_send_task(void *args)
{
    for (;;)
    {
        int64_t ms = core2_clock_bootms();

        for (size_t i = 0; i < tx_frames_count; i++)
        {
            if (tx_frames[i].next_send < ms)
            {
                tx_frames[i].next_send = ms + (int64_t)tx_frames[i].send_interval;
                core2_can_send_frame((const CAN_frame_t *)&tx_frames[i].frame);
            }
        }

        vPortYield();
    }

    vTaskDelete(NULL);
}

void core2_ecu_main(void *args)
{
    for (;;)
    {
        if (emu_online())
        {
        }

        vPortYield();
    }

    vTaskDelete(NULL);
}

void on_update(core2_update_event_t e)
{
    if (e == CORE2_EVENT_OTA_BEGIN)
        update_anim = true;
    else if (e == CORE2_EVENT_OTA_END)
        update_anim = false;
}

void core2_can_main()
{
    dprintf("Hello CAN World!\n");

    while (true)
    {
    }

    variables_init();

    // core2_gpio_set_output(PIN_5V_EN, CORE2_GPIO_MODE_NONE);
    // core2_gpio_write(PIN_5V_EN, HIGH);

    core2_can_init();

    dprintf("Adding wifi networks\n");
    core2_wifi_add_network("Barisic", "123456789");
    core2_wifi_add_network("Serengeti", "srgt#2018");
    core2_wifi_add_network("Tst", "123456789");

    dprintf("Starting tasks\n");
    int priority = 10;

    bool start_can = false;

    if (start_can)
    {
        xTaskCreate(core2_can_recv_task, "core2_can_recv", 1024 * 5, NULL, priority, NULL);
        xTaskCreate(core2_can_send_task, "core2_can_send", 1024 * 5, NULL, priority, NULL);
        xTaskCreate(core2_ecu_main, "core2_ecu", 1024 * 5, NULL, priority, NULL);
    }

    xTaskCreate(core2_ws2812fx_service, "ws2812fx.service", 1024 * 5, NULL, priority, NULL);
    core2_update_callback(on_update);

    // core2_update_start_from_file("/sd/firmware.bin");
    // dprintf("Firmware flash successful!\n");

    core2_http_start();
    core2_server_udp_start();

    while (!color_ready)
        vPortYield();

    int32_t seconds = 0;

    tx_frames[0].frame.MsgID = 0x400;
    tx_frames[0].frame.data.u8[0] = 0;
    tx_frames[0].frame.FIR.B.DLC = 8;
    tx_frames[0].send_interval = 20;
    // tx_frames_count++;

    for (;;)
    {
        seconds = core2_clock_bootseconds();

        while (update_anim)
        {
            ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(100, 0, 0)), 0, NO_OPTIONS);
            core2_sleep(10);
            continue;
        }

        ws2812fx.setSegment(0, 0, LED_COUNT - 1, FX_MODE_STATIC, COLORS(RGB(0, 100, 0)), 0, NO_OPTIONS);
        core2_sleep(100);
    }
}

#endif