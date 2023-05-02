#include <Arduino.h>

#include <SPI.h>
#include <Mcp320x.h>
#include <Wire.h>
#include "U8g2lib.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char *ssid = "Optima-34d393";
const char *password = "OPTIMA2701002212";

const char *host = "demo.sbarisic.com";
const int httpsPort = 443;

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

EventGroupHandle_t eventGroup;
const int gotVoltage = BIT0;
const int gotTempPress = BIT1;
const int gotLCD = BIT2;

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

    DynamicJsonDocument doc(512);

    doc["APIKey"] = "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw="; //    /vyDA/OWuDLOHMrbN+Dso7tONCfjiNRKnnNd4FqTEQA=";
    doc["Action"] = 1;
    doc["ACCvoltage1"] = voltage1;
    doc["ACCvoltage2"] = voltage2;
    doc["Tlak"] = gpressure;
    doc["Temperatura"] = gtemperature;

    String sendBuff;
    serializeJson(doc, sendBuff);

    WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect(host, 443))
    {
      Serial.println("Connection Failed");
      return;
    }
    /*
    uint8_t valid_fingerprint[32] = {0x90, 0xC4, 0x9F, 0xE3, 0xD4, 0x82, 0x78, 0xEE, 0x7E, 0x3, 0xD7, 0xE9, 0x1, 0x2C, 0x2D, 0x10, 0xCF, 0x8D, 0x27, 0x88, 0xF0, 0x20, 0x31, 0x4D, 0xF9, 0x51, 0xF8, 0xF8, 0xA4, 0x15, 0xBF, 0xC7 };
    uint8_t fingerprint[32];
    client.getFingerprintSHA256(fingerprint);
    Serial.print("Fingerprint: ");

    for (int i = 0; i < 32; i++) {
      Serial.print("0x");
      Serial.print(fingerprint[i], HEX);
      Serial.print(", ");
    }
    Serial.println();

    bool certificate_valid = true;
    for (int i = 0; i < 32; i++) {
      if (valid_fingerprint[i] != fingerprint[i]) {
        certificate_valid = false;
        break;
      }
    }

    if (certificate_valid) {
      Serial.println("Fingerprint valid");
    } else {
      Serial.println("Fingerprint invalid");
      return;
    }
    */
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

void setup()
{
  Serial.begin(115200);

  eventGroup = xEventGroupCreate();

  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);

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
      16384,
      NULL, // task parameters
      1,    // task priority
      NULL, // task handle
      1     // core 2
  );
}

void loop()
{
  // if( count1 > 3 && task1_handler != NULL)
  // vTaskSuspend(task1_handler);
}

// #include <freertos/freeRTOS.h>
// #include <freertos/task.h>
// include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <ArduinoJson.h>

// #ifndef STASSID
// #define STASSID "Optima-34d393"
// #define STAPSK  "OPTIMA2701002212"
// #endif

/*
const char* ssid = STAS SID;
const char* password = STAPSK;

const char* host = "demo.sbarisic.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "90 C4 9F E3 D4 82 78 EE 7E 03 D7 E9 01 2C 2D 10 CF 8D 27 88 F0 20 31 4D F9 51 F8 F8 A4 15 BF C7";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  //client.getFingerprintSHA256(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/deviceaccess";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: 148\r\n" +
               "User-Agent: TestESP32\r\n" +
               "Connection: close\r\n\r\n");

  client.print("{ \"APIVersion\": 1, \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"EquipmentKey\": \"7D-K0CG9rEW2iEwViKHqAg\", \"Value\": \"100.0\" }\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  Serial.println(line);
}

void loop() {
}
*/