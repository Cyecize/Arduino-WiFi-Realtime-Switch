#ifndef ARDUINO_WIFI_REALTIME_SWITCH_GENERAL_H
#define ARDUINO_WIFI_REALTIME_SWITCH_GENERAL_H

class General {

public:
    static const long SERIAL_BAUD_RATE = 115200;
    static const char *SERVER_NAME;
    static const int SERVER_PORT = 4200;
    static const char *SOCKET_URL;
    static const int SWITCH_PIN = 10;
    static const int WIFI_BAUD_RATE = 9600;
    static const int WIFI_RX_PIN = 6;
    static const int WIFI_TX_PIN = 7;
};

const char *General::SERVER_NAME = "cyecize.com";
const char *General::SOCKET_URL = "/socket";;

#endif //ARDUINO_WIFI_REALTIME_SWITCH_GENERAL_H
