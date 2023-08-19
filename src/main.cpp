#include <Arduino.h>
#include <WiFiEspAT.h>
#include <SoftwareSerial.h>
#include "constants/General.h"
#include "conn/WebSocketClient.h"
#include "execution/CommandExecutor.h"
#include "util/MemoryMonitor.h"
#include "util/TaskScheduler.h"

SoftwareSerial wifiSerial(General::WIFI_RX_PIN, General::WIFI_TX_PIN); // RX, TX
WiFiClient wifiClient;
WebSocketClient socketClient;
CommandExecutor executor;
MemoryMonitor memoryMonitor = MemoryMonitor();
TaskScheduler sockRestartScheduler;

void waitForWifi() {
    // waiting for connection to Wi-Fi network
    Serial.println("Waiting for connection to Wi-Fi");
    int c = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print('.');
        c++;

        if (c >= 30) {
            Serial.println();
        }
    }
    Serial.println();
    Serial.println("Connected to WiFi network.");
}

void setup() {
// write your initialization code here
    Serial.begin(General::SERIAL_BAUD_RATE);

    executor.init();
    while (!Serial);

    wifiSerial.begin(General::WIFI_BAUD_RATE);
    WiFi.init(wifiSerial);

    while (WiFi.status() == WL_NO_MODULE) {
        Serial.println();
        Serial.println("Communication with WiFi module failed, retrying indefinitely!");
        delay(1000);
    }

    waitForWifi();

    socketClient.init(
            wifiClient,
            General::SERVER_NAME,
            General::SERVER_PORT,
            General::SOCKET_URL,
            [](char *val) { executor.execute(val); }
    );

    sockRestartScheduler.init(
            General::SOCKET_RESTART_INTERVAL_MILLIS,
            true,
            []() { socketClient.restartConnection(); }

    );

    memoryMonitor.init();
}

void loop() {
    memoryMonitor.tick();
    sockRestartScheduler.tick();

    if (!socketClient.tick()) {
        //If the current task is timeout, finish it first, then fix the connection.
        if (executor.currentCommand() != TIMEOUT) {
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("Wi-Fi connection was lost!");
                waitForWifi();
            }

            socketClient.forceConnect();
            sockRestartScheduler.reset();
        }
    }

    executor.tick();
}