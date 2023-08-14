#ifndef ARDUINO_WIFI_REALTIME_SWITCH_COMMANDEXECUTOR_H
#define ARDUINO_WIFI_REALTIME_SWITCH_COMMANDEXECUTOR_H

#include "CommandType.h"
#include "constants/General.h"
#include <Arduino.h>

class CommandExecutor {
private:
    CommandType currentCmd = NONE;
    const int switchPin = General::SWITCH_PIN;

public:
    void init() {
        pinMode(this->switchPin, OUTPUT);
    }

    /**
     *
     * @param cmd - xml-like string eg. <c>2</c> or <c>3</c><to>12</to>
     */
    void execute(char *cmd) {
        String params = String(cmd);

        if (this->currentCmd != NONE) {
            Serial.println("Another task is running!");
            return;
        }
        CommandType cmdType = parseCommand(params);

        if (!getCommandTypeHandler(cmdType).init(params)) {
            Serial.println("Could not initialize command handler!");
            return;
        }
        this->currentCmd = cmdType;
    }

    void tick() {
        if (getCommandTypeHandler(this->currentCmd).tick(this->switchPin)) {
            this->currentCmd = NONE;
        }
    }

    CommandType &currentCommand() {
        return this->currentCmd;
    }

    void finishTask() {
        this->currentCmd = NONE;
    }

private:
    static CommandType parseCommand(String &params) {
        String input = params.substring(params.indexOf("<c>") + 3, params.indexOf("</c>"));

        for (int i = 0; i < NUM_COMMANDS; i++) {
            if (input.equals(commandStrings[i])) {
                return static_cast<CommandType>(i);
            }
        }

        Serial.print("Invalid command! ");
        Serial.println(input);
        return NONE; // Return a default value or an error code
    }

    // Function to handle the parsed command
    BaseCommandHandler &getCommandTypeHandler(CommandType commandType) {
        return commandHandlers[commandType]();
    }

};

#endif //ARDUINO_WIFI_REALTIME_SWITCH_COMMANDEXECUTOR_H
