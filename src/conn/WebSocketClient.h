#ifndef ARDUINO_WIFI_REALTIME_SWITCH_WEBSOCKETCLIENT_H
#define ARDUINO_WIFI_REALTIME_SWITCH_WEBSOCKETCLIENT_H

#include <WiFiEspAT.h>
#include "../lib/base64/base64.h"

typedef void (*CallbackFunction)(char *);

class WebSocketClient {

private:
    static const int MAX_RETRY_CONN_ATTEMPTS = 5;
    static const int RETRY_CONN_INTERVAL_MS = 1000;
    static const int MAX_MESSAGE_SIZE_BYTES = 32;
    WiFiClient client;
    const char *server = nullptr;
    int port = 0;
    const char *url = nullptr;
    CallbackFunction callback = nullptr;

    char MSG_RESP_BUFFER[MAX_MESSAGE_SIZE_BYTES] = {};

public:
    void init(WiFiClient &cln,
              const char *srv,
              int srvPort,
              const char *socketUrl,
              const CallbackFunction &callbackFunc) {
        this->client = cln;
        this->server = srv;
        this->port = srvPort;
        this->url = socketUrl;
        this->callback = callbackFunc;

        this->forceConnect();
    };

    bool tick() {
        if (!client.connected()) {
            client.stop();
            Serial.println("Connection with server ended unexpectedly.");

            return false;
        }

        if (client.available()) {
            readWebSocketData();
        }

        return true;
    }

    void forceConnect() {
        int count = 1;
        Serial.println("Connecting to server socket!");
        while (!this->connect() && count <= MAX_RETRY_CONN_ATTEMPTS) {
            count++;
            Serial.println("Retrying server socket connection!");
            delay(RETRY_CONN_INTERVAL_MS);
        }

        if (count > MAX_RETRY_CONN_ATTEMPTS) {
            Serial.println(
                    "Could not connect to server socket after " + String(MAX_RETRY_CONN_ATTEMPTS) + " attempts.");
        }
    }

private:
    bool connect() {
        if (!client.connect(server, 4200)) {
            return false;
        }


        // Line 1
        client.print("GET ");
        client.print(this->url);
        client.println(" HTTP/1.1");


        // Host header
        client.println("Host: cyecize.com:4200");
        client.print(this->server);
        if (this->port != 80) {
            client.print(":" + String(this->port));
        }
        client.println();

        client.println("Connection: Upgrade");
        client.println("Upgrade: websocket");

        // Websocket Key header
        client.print("Sec-WebSocket-Key: ");
        client.println(generateWebSocketKey());

        client.println("Sec-WebSocket-Version: 13");
        client.println();
        client.flush();

        readHTTPResponseHeaders();

        return true;
    }

    static String generateWebSocketKey() {
        unsigned char randomBytes[16];
        unsigned char key[17];
        for (unsigned char &randomByte: randomBytes) {
            randomByte = random(256); // Generate a random byte
        }

        // Encode randomBytes using base64
        size_t encodedLength = encode_base64(randomBytes, 17, key);
        key[encodedLength] = '\0'; // Null-terminate the string

        return {reinterpret_cast<char *>(key)};
    }

    void readHTTPResponseHeaders() {
        int currentChar;
        int prevChar = -1;
        int newLineCount = 0;

        while (true) {
            while (client.available() > 0) {
                currentChar = client.read();

                if (currentChar == -1) {
                    // End of stream
                    break;
                }

                Serial.print((char) currentChar);

                if (prevChar == '\r' && currentChar == '\n') {
                    newLineCount++;
                    if (newLineCount == 2) {
                        // End of HTTP response headers
                        break;
                    }
                } else if (currentChar != '\n' && currentChar != '\r') {
                    // Reset newLineCount if the character is not a newline
                    newLineCount = 0;
                }

                prevChar = currentChar;
            }

            if (newLineCount == 2 || currentChar == -1) {
                // End of HTTP response headers or end of stream, break out of the loop
                break;
            }
        }

        Serial.println("\nReceived HTTP Response Headers\n");
    }

    void readWebSocketData() {
        byte opcode, mask;
        unsigned int payloadLength;
        byte maskingKey[4];


        // Read the first two bytes (FIN, RSV1-3, Opcode, MASK, Payload Length)
        while (client.available() < 2); // Wait until at least 2 bytes are available
        byte b1 = client.read();
        byte b2 = client.read();

        bool finFlag = (b1 & 0b10000000) != 0;
        opcode = b2 & 0b00001111;
        mask = (opcode & 0b10000000) != 0;
        opcode &= 0b01111111;
        payloadLength = b2 & 0b01111111;

        if (payloadLength == 126) {
            byte lengthBytes[2];
            while (client.available() < 2); // Wait until at least 2 bytes are available
            client.readBytes(lengthBytes, 2);
            payloadLength = ((lengthBytes[0] & 0xFF) << 8) | (lengthBytes[1] & 0xFF);
        } else if (payloadLength == 127) {
            byte lengthBytes[8];
            while (client.available() < 8); // Wait until at least 8 bytes are available
            client.readBytes(lengthBytes, 8);
            unsigned long extendedLength = 0;
            for (int i = 0; i < 8; i++) {
                extendedLength |= (unsigned long) (lengthBytes[i] & 0xFF) << ((7 - i) * 8);
            }
            payloadLength = (unsigned int) (extendedLength & 0x7FFFFFFFFFFFFFFF);
        }

        // Read the optional masking key
        if (mask) {
            while (client.available() < 4); // Wait until at least 4 bytes are available
            client.readBytes(maskingKey, 4);
        }

        // Read the payload data
        // while (wifiClient.available() < payloadLength); // Wait until payloadLength bytes are available
        client.readBytes(MSG_RESP_BUFFER, min(MAX_MESSAGE_SIZE_BYTES, payloadLength));
        if (payloadLength > MAX_MESSAGE_SIZE_BYTES) {
            Serial.println(
                    "Socket message larger than " + String(MAX_MESSAGE_SIZE_BYTES) + " bytes, trimming the rest!");
            size_t diff = payloadLength - MAX_MESSAGE_SIZE_BYTES;
            for (size_t i = 0; i < diff; i++) {
                client.read();
            }

            payloadLength = MAX_MESSAGE_SIZE_BYTES;
        }

        // Unmask payload data if necessary
        if (mask) {
            for (unsigned int i = 0; i < payloadLength; i++) {
                MSG_RESP_BUFFER[i] = MSG_RESP_BUFFER[i] ^ maskingKey[i % 4];
            }
        }

        this->callback(MSG_RESP_BUFFER);

        // Reset the buffer.
        memset(MSG_RESP_BUFFER, 0, MAX_MESSAGE_SIZE_BYTES);
    }
};


#endif //ARDUINO_WIFI_REALTIME_SWITCH_WEBSOCKETCLIENT_H
