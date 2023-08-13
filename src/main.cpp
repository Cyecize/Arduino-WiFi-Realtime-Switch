#include <Arduino.h>
#include <WiFiEspAT.h>
#include <SoftwareSerial.h>
#include "constants/General.h"
#include "conn/WebSocketClient.h"

SoftwareSerial wifiSerial(General::WIFI_RX_PIN, General::WIFI_TX_PIN); // RX, TX
WiFiClient wifiClient;
WebSocketClient socketClient;

void setup() {
// write your initialization code here
    Serial.begin(General::SERIAL_BAUD_RATE);

    pinMode(General::SWITCH_PIN, OUTPUT);
    while (!Serial);

    wifiSerial.begin(General::WIFI_BAUD_RATE);
    WiFi.init(wifiSerial);

    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println();
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        while (true);
    }

    // waiting for connection to Wi-Fi network
    Serial.println("Waiting for connection to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print('.');
    }
    Serial.println();
    Serial.println("Connected to WiFi network.");

    socketClient.init(
            wifiClient,
            General::SERVER_NAME,
            General::SERVER_PORT,
            General::SOCKET_URL,
            [](char *val) {
                Serial.print("I got ");
                Serial.println(*val);
            }
    );

}

void loop() {
   socketClient.tick();
}