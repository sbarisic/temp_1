#include <core2.h>

#if !defined(CORE2_DISABLE_MCP320X)
#include <Mcp320x.h>

SPISettings spi_settings;
SemaphoreHandle_t lock = NULL;

#define ADC_VREF 3299 // 3.3V Vref

// MCP3201 adc(MCP320X_ADC_VREF, MCP320X_CS_CHANNEL1); // ovisno o tipu AD konvertera MCP 3201,3202,3204,3208
// MCP3201 adc1(MCP320X_ADC_VREF, MCP320X_CS_CHANNEL2);

// MCP3201 adc(ADC_VREF, 5); // ovisno o tipu AD konvertera MCP 3201,3202,3204,3208
// MCP3201 adc1(ADC_VREF, 17);
// MCP3201 adcc(ADC_VREF, 4);

#endif

void core2_adc_chipselect_enable(core2_adc_channel_t Ch)
{
    if (Ch & CORE2_ADC_CH1)
        digitalWrite(MCP320X_CS_CHANNEL1, HIGH);

    if (Ch & CORE2_ADC_CH2)
        digitalWrite(MCP320X_CS_CHANNEL2, HIGH);
}

void core2_adc_chipselect_enable_all()
{
    digitalWrite(MCP320X_CS_CHANNEL1, HIGH);
    digitalWrite(MCP320X_CS_CHANNEL2, HIGH);
}

void core2_adc_chipselect_disable(core2_adc_channel_t Ch)
{
    if (Ch & CORE2_ADC_CH1)
        digitalWrite(MCP320X_CS_CHANNEL1, LOW);

    if (Ch & CORE2_ADC_CH2)
        digitalWrite(MCP320X_CS_CHANNEL2, LOW);
}

void core2_adc_chipselect_disable_all()
{
    digitalWrite(MCP320X_CS_CHANNEL1, LOW);
    digitalWrite(MCP320X_CS_CHANNEL2, LOW);
}

bool core2_adc_lock()
{
    dprintf("core2_adc_lock() - ");

    if (core2_lock_begin(lock))
    {
        dprintf("TRUE\n");
        return true;
    }

    dprintf("FALSE\n");
    return false;
}

void core2_adc_unlock(bool was_locked)
{
    dprintf("core2_adc_unlock(%d)\n", was_locked ? 1 : 0);

    if (was_locked)
        core2_lock_end(lock);
}

/*
void core2_adc_read_ex(float *VoltArray, float *Factors, core2_adc_channel_t Ch, bool UseLock)
{
#if defined(CORE2_DISABLE_MCP320X)
    {
        VoltArray[0] = -1.00f;
        return;
    }
#else

    bool Locked = !UseLock;
    int ChIdx = -1;

    if (UseLock)
        Locked = core2_adc_lock();

    if (Locked)
    {
        core2_adc_chipselect_enable(Ch);
        SPI.beginTransaction(spi_settings);

        uint16_t raw = 0;
        uint16_t analog = 0;

        if (Ch & CORE2_ADC_CH1)
        {
            raw = adc.read(MCP3201::Channel::SINGLE_0);
            analog = adc.toAnalog(raw);

            ChIdx++;
            VoltArray[ChIdx] = (float)analog * (Factors == NULL ? 1 : Factors[ChIdx]);
        }

        if (Ch & CORE2_ADC_CH2)
        {
            raw = adc1.read(MCP3201::Channel::SINGLE_0);
            analog = adc1.toAnalog(raw);

            ChIdx++;
            VoltArray[ChIdx] = (float)analog * (Factors == NULL ? 1 : Factors[ChIdx]);
        }

        SPI.endTransaction();
        core2_adc_chipselect_disable(Ch);

        if (UseLock && Locked)
        {
            core2_adc_unlock(Locked);
        }
    }

#endif
}
*/

#define ADC_VREF 3299   // 3.3V Vref
#define ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz
SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);

MCP3201 adc(ADC_VREF, 5); // ovisno o tipu AD konvertera MCP 3201,3202,3204,3208
MCP3201 adc1(ADC_VREF, 17);
MCP3201 adcc(ADC_VREF, 4);

void core2_adc_init2()
{
    SPI.begin();
    SPI.beginTransaction(settings);
}

void core2_adc_read2(float *voltage1, float *voltage2, float *current)
{
    uint16_t raw = adc.read(MCP3201::Channel::SINGLE_0);
    uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);

    // get analog value
    uint16_t val = adc.toAnalog(raw);
    uint16_t val1 = adc1.toAnalog(raw1);

    int voltagesum = 0;
    for (int i = 0; i < 50; i++)
    {
        uint16_t raw2 = adcc.read(MCP3201::Channel::SINGLE_0);
        uint16_t valc = adcc.toAnalog(raw2);
        voltagesum = voltagesum + int(valc);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    dprintf("raw = %d, ", raw);
    dprintf("raw1 = %d, ", raw1);
    dprintf("val = %d, ", val);
    dprintf("val1 = %d\n", val1);

    *voltage1 = val * 7.34 / 1000;
    *voltage2 = val1 * 13.75 / 1000 - val * 7.34 / 1000;
    *current = (voltagesum / 50) * 2;
}

void core2_adc_read(float *Volt1, float *Volt2, float *Current)
{
#if !defined(CORE2_DISABLE_MCP320X)
    dprintf("core2_adc_read()\n");

    if (Volt1 == NULL || Volt2 == NULL || Current == NULL)
    {
        return;
    }

    *Volt1 = 0;
    *Volt2 = 0;
    *Current = 0;

    if (core2_lock_begin(lock))
    {
        core2_adc_chipselect_enable_all();
        SPI.beginTransaction(spi_settings);

        //---------------------------------------------------------
        uint16_t raw = adc.read(MCP3201::Channel::SINGLE_0);
        uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);

        // get analog value
        uint16_t val = adc.toAnalog(raw);
        uint16_t val1 = adc1.toAnalog(raw1);

        int voltagesum = 0;
        for (int i = 0; i < 50; i++)
        {
            uint16_t raw2 = adcc.read(MCP3201::Channel::SINGLE_0);
            uint16_t valc = adcc.toAnalog(raw2);
            voltagesum = voltagesum + int(valc);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        *Volt1 = val * 7.34f / 1000;
        *Volt2 = val1 * 13.75f / 1000 - val * 7.34 / 1000;
        *Current = (voltagesum / 50) * 2;

        //---------------------------------------------------------

        SPI.endTransaction();
        core2_adc_chipselect_disable_all();

        core2_lock_end(lock);
    }
#else
    *Volt1 = 0;
    *Volt2 = 0;
#endif
}

bool core2_mcp320x_init()
{
#if defined(CORE2_DISABLE_MCP320X)
    dprintf("core2_mcp320x_init() - SKIPPING, DISABLED\n");
    return false;
#else

    dprintf("core2_mcp320x_init()\n");
    lock = core2_lock_create();

    pinMode(5, OUTPUT);
    pinMode(17, OUTPUT);
    pinMode(4, OUTPUT);

    pinMode(MCP320X_CS_CHANNEL1, OUTPUT);
    pinMode(MCP320X_CS_CHANNEL2, OUTPUT);

    spi_settings = SPISettings(MCP320X_ADC_CLK, MSBFIRST, SPI_MODE0);
    SPI.begin(MCP320X_PIN_CLK, MCP320X_PIN_MISO, MCP320X_PIN_MOSI, MCP320X_PIN_CS);

    // configure PIN mode

    return true;
#endif
}