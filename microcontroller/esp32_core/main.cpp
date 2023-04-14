/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "Barisic"
#define STAPSK  "123456789"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "demo.sbarisic.com";
const int httpsPort = 443;

// Use web browser to view and copy SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "1B 84 53 64 6E 7C B0 38 7E 56 2C 81 DE 5C C0 B4 59 3A 9C 3D";

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
}

void send_data(float DataVal) {
  DynamicJsonDocument doc(512);
  //StaticJsonDocument<512> doc;

  doc["APIVersion"] = 1;
  doc["APIKey"] = "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw="; //    /vyDA/OWuDLOHMrbN+Dso7tONCfjiNRKnnNd4FqTEQA=";
  doc["Action"] = 1;
  doc["EquipmentKey"] = "7D-K0CG9rEW2iEwViKHqAg";
  doc["Value"] = String("") + DataVal;
  
  String sendBuff;
  serializeJson(doc, sendBuff);

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection Failed");
    return;
  }

  String url = "/deviceaccess";

  String httpHeader = "POST ";
  httpHeader = httpHeader + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + sendBuff.length() + "\r\n" +
    "User-Agent: TestESP8266\r\n" +
    "Connection: close\r\n\r\n";


  Serial.println("HEADER:");
  Serial.println(httpHeader);

  Serial.println("JSON:");
  Serial.println(sendBuff);


  client.print(httpHeader);

  //client.print("{ \"APIVersion\": 1, \"APIKey\": \"OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=\", \"Action\": 1, \"EquipmentKey\": \"7D-K0CG9rEW2iEwViKHqAg\", \"Value\": \"8.0\" }\r\n");
  client.print(sendBuff);
  client.print("\r\n");

  Serial.println("Request Sent");
  while (client.connected()) {
    /*String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }*/


  }

  Serial.println(client.readString());

  //String line = client.readStringUntil('\n');
  //Serial.println(line);
}

void loop() {
  for (int i = 7; i < 15; i++) {
    delay(2500); 
    send_data(i);
  }
}
