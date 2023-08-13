#include <Arduino.h>
#include <WiFiEspAT.h>
#include <SoftwareSerial.h>
#include "constants/General.h"
#include "conn/WebSocketClient.h"

SoftwareSerial wifiSerial(General::WIFI_RX_PIN, General::WIFI_TX_PIN); // RX, TX
WiFiClient wifiClient;
WebSocketClient socketClient;

void waitForWifi() {
    // waiting for connection to Wi-Fi network
    Serial.println("Waiting for connection to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print('.');
    }
    Serial.println();
    Serial.println("Connected to WiFi network.");
}

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

    waitForWifi();

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
    if (!socketClient.tick()) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi connection was lost!");
            waitForWifi();
        }

        socketClient.forceConnect();
    }
}