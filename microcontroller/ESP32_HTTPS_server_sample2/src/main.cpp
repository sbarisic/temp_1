#include <Arduino.h>

#include <SPI.h>
#include <Mcp320x.h>
#include <Wire.h>
#include "U8g2lib.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// #define SOC_SDMMC_USE_GPIO_MATRIX
#include "driver/sdmmc_host.h"

// Filesystem
bool core2_filesystem_init();
FILE *core2_file_open(const char *filename, const char *type = NULL);
bool core2_file_close(FILE *f);
bool core2_file_move(const char *oldf, const char *newf);
bool core2_file_write(const char *filename, const char *data, size_t len);
bool core2_file_mkdir(const char *dirname, mode_t mode = 0);
void core2_file_list(const char *dirname);

const char *ssid = "Optima-34d393_EXT";
const char *password = "OPTIMA2701002212";

const char *host = "demo.sbarisic.com";
const int httpsPort = 443;

#define PIN_SWITCH (gpio_num_t)15 // !!!
QueueHandle_t interruptQueue;

// define for MCP3201
// #define SPI_CS      4        // SPI slave select
// #define SPI_CS1     5
#define ADC_VREF 3311   // 3.3V Vref
#define ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

uint8_t id = 0x28;                   // i2c address
uint8_t data[7];                     // holds output data
uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // command to be sent
double press_counts = 0;             // digital pressure reading [counts]
double temp_counts = 0;              // digital temperature reading [counts]
// double pressure = 0; // pressure reading [bar, psi, kPa, etc.]
// double temperature = 0; // temperature reading in deg C
double outputmax = 15099494; // output at maximum pressure [counts]
double outputmin = 1677722;  // output at minimum pressure [counts]
double pmax = 10;            // maximum value of pressure range [bar, psi, kPa, etc.]
double pmin = 0;             // minimum value of pressure range [bar, psi, kPa, etc.]

MCP3201 adc(ADC_VREF, 4); // ovisno o tipu AD konvertera MCP 3201,3202,3204,3208
MCP3201 adc1(ADC_VREF, 5);

// define LCD SSD1306
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g(U8G2_R0, U8X8_PIN_NONE);

// Global variable
float voltage1;
float voltage2;
float gpressure;
float gtemperature;
char string[8];
float naponi[200]; // marica za zapis padova napona u interruptu
volatile int naponi_ready;

volatile bool buzzer_enable = false;

EventGroupHandle_t eventGroup;
const int gotVoltage = BIT0;
const int gotTempPress = BIT1;
const int gotLCD = BIT2;
const int gotVoltageDrop = BIT3;

static void gpio_isr_handler(void *args) // definicija ISR
{
  int pinNumber = (int)args;
  xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
}

bool leds_initialized = false;

void led_enable(bool r, bool g, bool b)
{
  if (!leds_initialized)
  {
    pinMode(13, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(12, OUTPUT);
  }

  digitalWrite(14, r ? HIGH : LOW);
  digitalWrite(12, g ? HIGH : LOW);
  digitalWrite(13, b ? HIGH : LOW);
}

void buttonPushedTask(void *params) // task koji se pokreće nakon aktivacije ISR
{
  int pinNumber, count = 0;
  while (true)
  {
    if (xQueueReceive(interruptQueue, &pinNumber, portMAX_DELAY))
    {
      gpio_num_t gpinNumber = (gpio_num_t)pinNumber;

      // disable the interrupt
      gpio_isr_handler_remove(gpinNumber);

      // wait some time while we check for the pin to be released
      do
      {
        vTaskDelay(5 / portTICK_PERIOD_MS);
      } while (gpio_get_level(gpinNumber) == (gpio_num_t)0);

      printf("GPIO %d was pressed %d times. The state is %d \n", pinNumber, count++, gpio_get_level(gpinNumber));

      // TODO: spremiti u memoriju
      // digitalWrite(14, HIGH);
      // delay(3000); // vTaskDelay(3000 / portTICK_PERIOD_MS);
      // digitalWrite(14, LOW);

      for (int i = 0; i < 200; i++)
      {
        float minnapon = 35.0;
        for (int j = 0; j < 20; j++)
        {
          uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);
          uint16_t val1 = adc1.toAnalog(raw1);
          float naponj = val1 * 9.215 / 1000;
          delay(1);
          if (naponj < minnapon)
            minnapon = naponj;
        }
        naponi[i] = int(minnapon * 100);
      }

      naponi_ready = 1;

      // vTaskDelay(1000 / portTICK_PERIOD_MS);

      for (int i = 0; i < 200; i++)
      {
        // printf(" %f , ", naponi[i]);
        Serial.print(int(naponi[i]));
        Serial.print(",");
      }
      printf("\n");

      digitalWrite(13, LOW);

      xEventGroupSetBits(eventGroup, gotVoltageDrop); // javljamo da su padovi napona očitani
      gpio_isr_handler_add(gpinNumber, gpio_isr_handler, (void *)pinNumber);
    }
  }
}

void task1(void *parameters)
{
  while (true)
  {
    printf("Task1\n");

    // sampling voltages every 2 sec

    uint16_t raw = adc.read(MCP3201::Channel::SINGLE_0);
    uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0);

    // get analog value
    uint16_t val = adc.toAnalog(raw);
    uint16_t val1 = adc1.toAnalog(raw1);

    voltage1 = val * 4.795 / 1000;
    voltage2 = val1 * 9.215 / 1000 - val * 4.795 / 1000;

    xEventGroupSetBits(eventGroup, gotVoltage); // javljamo da su naponi očitani

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void task2(void *parameters)
{
  while (true)
  {
    printf("Task2\n");

    Wire.beginTransmission(id);
    int stat = Wire.write(cmd, 3); // write command to the sensor
    stat |= Wire.endTransmission();

    delay(10); // vTaskDelay(10 / portTICK_PERIOD_MS);

    Wire.requestFrom(id, (uint8_t)7); // read back Sensor data 7 bytes

    for (int i = 0; i < 7; i++)
    {
      data[i] = Wire.read();
    }

    // start read pressure & temperature
    press_counts = data[3] + data[2] * 256 + data[1] * 65536; // calculate digital pressure counts
    temp_counts = data[6] + data[5] * 256 + data[4] * 65536;  // calculate digital temperature counts
    gtemperature = (temp_counts * 200 / 16777215) - 50;       // calculate temperature in deg c

    // calculation of pressure value according to equation 2 of datasheet
    gpressure = ((press_counts - outputmin) * (pmax - pmin)) / (outputmax - outputmin) + pmin;

    xEventGroupSetBits(eventGroup, gotTempPress); // dobili temperaturu i tlak

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void task3(void *pvparameter)
{
  while (true)
  {
    u8g.firstPage();
    do
    {
      u8g.drawRFrame(0, 0, 128, 64, 0);
      u8g.setFont(u8g_font_helvB12);

      u8g.drawStr(6, 14, "Acc 1: ");
      u8g.drawStr(65, 14, dtostrf(voltage1, 5, 2, string));
      u8g.drawStr(105, 14, "V");

      u8g.drawStr(6, 28, "Acc 2: ");
      u8g.drawStr(65, 28, dtostrf(voltage2, 5, 2, string));
      u8g.drawStr(105, 28, "V");

      u8g.drawStr(6, 42, "Press: ");
      u8g.drawStr(65, 42, dtostrf(gpressure, 5, 2, string));
      u8g.drawStr(105, 42, "Bar");

      u8g.drawStr(6, 56, "Temp : ");
      u8g.drawStr(65, 56, dtostrf(gtemperature, 5, 2, string));
      u8g.drawStr(105, 56, "C");

    } while (u8g.nextPage());

    xEventGroupSetBits(eventGroup, gotLCD); // poslali na LCD

    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void task4(void *pvparameter)
{
  while (true)
  {
    printf("Task4\n");

    xEventGroupWaitBits(eventGroup, gotVoltage | gotTempPress | gotLCD, true, true, portMAX_DELAY);

    DynamicJsonDocument doc(1024 * 3);

    doc["APIKey"] = "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw="; //    /vyDA/OWuDLOHMrbN+Dso7tONCfjiNRKnnNd4FqTEQA=";
    doc["Action"] = 1;
    doc["ACCvoltage1"] = voltage1;
    doc["ACCvoltage2"] = voltage2;
    doc["Tlak"] = gpressure;
    doc["Temperatura"] = gtemperature;
    // doc["Test"] = naponi;

    if (naponi_ready)
    {
      Serial.println("Saljem napone!");

      for (int i = 0; i < 200; i++)
      {
        doc["Test"][i] = serialized(String(naponi[i] / 100, 2));
      }

      naponi_ready = 0;
    }

    String sendBuff;
    serializeJson(doc, sendBuff);

    WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect(host, 443))
    {
      Serial.println("Connection Failed");
      return;
    }

    uint8_t valid_fingerprint[32] = {0x90, 0xC4, 0x9F, 0xE3, 0xD4, 0x82, 0x78, 0xEE, 0x7E, 0x3, 0xD7, 0xE9, 0x1, 0x2C, 0x2D, 0x10, 0xCF, 0x8D, 0x27, 0x88, 0xF0, 0x20, 0x31, 0x4D, 0xF9, 0x51, 0xF8, 0xF8, 0xA4, 0x15, 0xBF, 0xC7};
    uint8_t fingerprint[32];
    client.getFingerprintSHA256(fingerprint);
    Serial.print("Fingerprint: ");

    for (int i = 0; i < 32; i++)
    {
      Serial.print("0x");
      Serial.print(fingerprint[i], HEX);
      Serial.print(", ");
    }
    Serial.println();

    bool certificate_valid = true;
    for (int i = 0; i < 32; i++)
    {
      if (valid_fingerprint[i] != fingerprint[i])
      {
        certificate_valid = false;
        break;
      }
    }

    if (certificate_valid)
    {
      Serial.println("Fingerprint valid");
    }
    else
    {
      Serial.println("Fingerprint invalid, testing");
      // return;
    }

    digitalWrite(13, HIGH);
    String url = "/deviceaccess"; // "/deviceaccess_dev";

    String httpHeader = "POST ";

    httpHeader = httpHeader + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + sendBuff.length() + "\r\n" +
                 "User-Agent: TestESP8266\r\n" +
                 "Connection: close\r\n";

    Serial.print("Length: ");
    Serial.println(sendBuff.length());
    Serial.print("JSON: ");
    Serial.println(sendBuff);

    client.println(httpHeader);

    client.print(sendBuff);
    client.print("\r\n");

    Serial.println("Request Sent");

    while (client.connected())
    {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
        Serial.println("headers received");
        break;
      }
    }

    Serial.print("Response:");
    Serial.println(client.readStringUntil('\n'));
    Serial.println(client.readStringUntil('\n'));

    // String line = client.readStringUntil('\n');
    // Serial.println(line);
    digitalWrite(13, LOW);

    vTaskDelay(15000 / portTICK_PERIOD_MS);
  }
}

void buzzer(void *unused)
{
  printf("Starting buzzer\n");
  pinMode(GPIO_NUM_25, OUTPUT);
  digitalWrite(GPIO_NUM_25, LOW);

  while (1)
  {
    if (buzzer_enable)
    {
      tone(GPIO_NUM_25, 3800, 1000);
      vTaskDelay(pdMS_TO_TICKS(1000));
      noTone(GPIO_NUM_25);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  vTaskDelete(NULL);
}

void setup()
{
  naponi_ready = 0;
  xTaskCreate(buzzer, "buzzer", 1024, NULL, 0, NULL);

  // definicija GPIO kao ISR
  gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
  gpio_pulldown_en(PIN_SWITCH);
  gpio_pullup_dis(PIN_SWITCH);
  gpio_set_intr_type(PIN_SWITCH, GPIO_INTR_NEGEDGE);

  pinMode(13, OUTPUT);

  interruptQueue = xQueueCreate(1, sizeof(int));                       // postavljen queue za interrupt
  xTaskCreate(buttonPushedTask, "button pushed", 2048, NULL, 1, NULL); // task koji se izvršava po interruptu

  gpio_install_isr_service(0);                                            // instalacija ISR-a
  gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH); // dodjela ISR

  Serial.begin(115200);

  eventGroup = xEventGroupCreate();

  // pinMode(13, OUTPUT);
  // pinMode(14, OUTPUT);
  // pinMode(12, OUTPUT);

  buzzer_enable = true;
  led_enable(true, false, false);
  vTaskDelay(pdMS_TO_TICKS(1000));

  led_enable(false, true, false);
  vTaskDelay(pdMS_TO_TICKS(1000));

  led_enable(false, false, true);

  vTaskDelay(pdMS_TO_TICKS(1000));
  led_enable(false, false, false);
  buzzer_enable = false;

  // I2C
  Wire.begin();

  // LCD begin
  u8g.begin();

  // configure PIN mode
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  // set initial PIN state
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);

  // initialize SPI interface for MCP3208 & LCD
  SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
  SPI.begin();
  SPI.beginTransaction(settings);

  Serial.begin(115200);
  Serial.println();
  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());

  core2_filesystem_init();

  xTaskCreatePinnedToCore(
      task1,
      "Task 1",
      2048,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      0     // core 1
  );
  xTaskCreatePinnedToCore(
      task2,
      "Task 2",
      2048,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      0     // core 1
  );

  xTaskCreatePinnedToCore(
      task3,
      "Task 3",
      2048,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      0     // core 2
  );

  xTaskCreatePinnedToCore(
      task4,
      "Task 4",
      4096 * 2,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      1     // core 2
  );

  xTaskCreatePinnedToCore(
      buttonPushedTask,
      "Task 5",
      4096,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      0     // core 1
  );
}

void loop()
{
  // if( count1 > 3 && task1_handler != NULL)
  // vTaskSuspend(task1_handler);
}

/////////////////////////// SD CARD

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO GPIO_NUM_33 // GPIO_NUM_2
#define PIN_NUM_MOSI GPIO_NUM_26 // GPIO_NUM_15
#define PIN_NUM_CLK GPIO_NUM_32  // GPIO_NUM_14
#define PIN_NUM_CS GPIO_NUM_23   // GPIO_NUM_13

#define SDCARD_PIN_MOSI GPIO_NUM_26 //
#define SDCARD_PIN_MISO GPIO_NUM_33 //
#define SDCARD_PIN_CLK GPIO_NUM_32  //
#define SDCARD_PIN_CS GPIO_NUM_23   //

#define dprintf printf

#undef ESP_LOGI
#define ESP_LOGI(a, ...)  \
  do                      \
  {                       \
    dprintf(__VA_ARGS__); \
    dprintf("\n");        \
  } while (0)

FILE *core2_file_open(const char *filename, const char *type)
{
  FILE *f = NULL;

  if (type == NULL)
  {
    dprintf("core2_file_open(\"%s\", \"w\")", filename);
    f = fopen(filename, "w");
  }
  else
  {
    dprintf("core2_file_open(\"%s\", \"%s\")", filename, type);
    f = fopen(filename, type);
  }
  dprintf(" - %p\n", (void *)f);
  return f;
}

bool core2_file_close(FILE *f)
{
  dprintf("core2_file_close(%p)\n", (void *)f);
  if (!fclose(f))
    return true;

  return false;
}

bool core2_file_move(const char *oldf, const char *newf)
{
  if (rename(oldf, newf) == -1)
    return false;

  return true;
}

bool core2_file_write(const char *filename, const char *data, size_t len)
{
  dprintf("core2_write_file(\"%s\") - ", filename);

  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    dprintf("FAIL\n");
    return false;
  }

  fwrite(data, 1, len, f);
  fclose(f);

  dprintf("OK\n");
  return true;
}

bool core2_file_append(const char *filename, const char *data, size_t len)
{
  dprintf("core2_write_append(\"%s\") - ", filename);

  FILE *f = fopen(filename, "a");
  if (f == NULL)
  {
    dprintf("FAIL\n");
    return false;
  }

  fwrite(data, 1, len, f);
  fclose(f);

  dprintf("OK\n");
  return true;
}

bool core2_file_mkdir(const char *dirname, mode_t mode)
{
  if (mode == 0)
    mode = 0777;

  dprintf("core2_file_mkdir(\"%s\", %o) - ", dirname, mode);

  if (mkdir(dirname, mode) == -1)
  {
    dprintf("FAIL\n");
    return false;
  }

  dprintf("OK\n");
  return true;
}

void core2_file_list(const char *dirname)
{
  struct dirent *dir;
  DIR *d = opendir(dirname);

  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      dprintf("FOUND: %s\n", dir->d_name);
    }

    closedir(d);
  }
}

bool core2_filesystem_init()
{
  dprintf("core2_filesystem_init() - Initializing SD card, using SPI\n");

  esp_err_t ret;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};
  sdmmc_card_t *card;

  const char mount_point[] = "/sd";

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

  if (ret != ESP_OK)
  {
    dprintf("core2_filesystem_init() - Failed to initialize bus\n");
    return false;
  }

  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = PIN_NUM_CS;
  slot_config.host_id = (spi_host_device_t)host.slot;

  dprintf("core2_filesystem_init() - Mounting filesystem\n");
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK)
  {
    if (ret == ESP_FAIL)
      dprintf("Failed to mount filesystem. If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option\n");
    else
      dprintf("Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place\n", esp_err_to_name(ret));

    return false;
  }

  dprintf("core2_filesystem_init() - Filesystem mounted @ \"%s\"\n", mount_point);
  sdmmc_card_print_info(stdout, card);

  dprintf("core2_filesystem_init() - Creating folders\n");
  core2_file_mkdir("/sd/logs");       // Log directory
  core2_file_mkdir("/sd/processing"); // Temporary processing directory, files which have not been uploaded?

  return true;
}