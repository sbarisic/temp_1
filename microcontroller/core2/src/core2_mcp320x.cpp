#include <core2.h>
#include <Mcp320x.h>

#define ADC_VREF 3311   // 3.3V Vref
#define ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

MCP3201 adc(ADC_VREF, 4); // ovisno o tipu AD konvertera MCP 3201,3202,3204,3208
MCP3201 adc1(ADC_VREF, 5);

void core2_adc_read(float *Volt1, float *Volt2)
{
    dprintf("SPISettings\n");

    // initialize SPI interface for MCP3208 & LCD
    SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);

    dprintf("SPI.begin\n");
    SPI.begin();

    dprintf("SPI.beginTransaction\n");
    SPI.beginTransaction(settings);

    dprintf("adc.read\n");
    //---------------------------------------------------------
    uint16_t raw = adc.read(MCP3201::Channel::SINGLE_0);
    uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);

    // get analog value
    uint16_t val = adc.toAnalog(raw);
    uint16_t val1 = adc1.toAnalog(raw1);

    float voltage1 = val * 4.795 / 1000;
    float voltage2 = val1 * 9.215 / 1000 - val * 4.795 / 1000;

    *Volt1 = voltage1;
    *Volt2 = voltage2;

    dprintf("SPI.end\n");

    //-------------
    SPI.end();
}

bool core2_mcp320x_init()
{
    /*dprintf("core2_mcp320x_init()\n");

    // configure PIN mode
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    // set initial PIN state
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);

    // initialize SPI interface for MCP3208 & LCD
    SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
    SPI.begin();
    SPI.beginTransaction(settings);*/

    // configure PIN mode
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    // set initial PIN state
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);

    return true;
}