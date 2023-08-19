#ifndef ARDUINO_WIFI_REALTIME_SWITCH_COMMANDTYPE_H
#define ARDUINO_WIFI_REALTIME_SWITCH_COMMANDTYPE_H

#define COMMAND_TYPES(X) \
    X(NONE, "0", handleNone) \
    X(ON, "1", handleOn) \
    X(OFF, "2", handleOff) \
    X(TIMEOUT, "3", handleTimeout)

class BaseCommandHandler {
public:
    virtual bool init(String &params) { return true; }

    virtual bool tick(int outputPin) = 0;
};

class OnOffHandler : public BaseCommandHandler {
private:
    int state;
public:
    explicit OnOffHandler(int state) : state(state) {}

private:
    bool tick(int outputPin) override {
        digitalWrite(outputPin, this->state);
        Serial.print("Set state ");
        Serial.println(this->state);
        return true;
    }
};

class NoneHandler : public BaseCommandHandler {
    bool tick(int outputPin) override {
        return false;
    }
};

class TimeoutHandler : public BaseCommandHandler {
private:
    unsigned long millisStart;
    long expectedDiff;
public:
    bool init(String &params) override {
        // Eg. params: "<c>3</c><to>15</to>"
        long time = params.substring(params.indexOf("<to>") + 4, params.indexOf("</to>")).toInt();
        if (time <= 0 || time > General::MAX_ALLOWED_TIMEOUT_SECONDS) {
            Serial.println("Invalid timeout value in command: " + params + "!");
            return false;
        }

        Serial.println("Setting timeout for " + String(time) + " seconds.");
        this->expectedDiff = time * 1000;
        this->millisStart = millis();

        return true;
    }

    bool tick(int outputPin) override {
        unsigned long now = millis();
        if (now - this->millisStart >= this->expectedDiff) {
            digitalWrite(outputPin, LOW);
            Serial.println("Timeout completed!");
            return true;
        }

        if (this->millisStart > now) {
            Serial.println("You happened to catch the millis overflowing after 49 days, lucky you!");
            return true;
        }

        digitalWrite(outputPin, HIGH);
        return false;
    }
};

NoneHandler noneHandler = NoneHandler();
OnOffHandler onHandler = OnOffHandler(1);
OnOffHandler offHandler = OnOffHandler(0);
TimeoutHandler timeoutHandler = TimeoutHandler();

// Define function prototypes for handlers
BaseCommandHandler &handleNone() { return noneHandler; }

BaseCommandHandler &handleOn() { return onHandler; }

BaseCommandHandler &handleOff() { return offHandler; }

BaseCommandHandler &handleTimeout() { return timeoutHandler; }

// Define the handler function pointers
typedef BaseCommandHandler &(*CommandHandler)();

// Helper macro to define enum, string, and handler entries
#define DEFINE_COMMAND_ENUM_HANDLER(enumerator, string, handler) enumerator,
#define DEFINE_COMMAND_STRING(enumerator, string, handler) string,
#define DEFINE_COMMAND_HANDLER(enumerator, string, handler) handler,

// Generate enum, string array, and handler array
enum CommandType {
    COMMAND_TYPES(DEFINE_COMMAND_ENUM_HANDLER)
    NUM_COMMANDS
};
const char *commandStrings[] = {
        COMMAND_TYPES(DEFINE_COMMAND_STRING)
};

CommandHandler commandHandlers[] = {
        COMMAND_TYPES(DEFINE_COMMAND_HANDLER)
};


#endif //ARDUINO_WIFI_REALTIME_SWITCH_COMMANDTYPE_H
