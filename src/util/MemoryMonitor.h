#ifndef ARDUINO_WIFI_REALTIME_SWITCH_MEMORYMONITOR_H
#define ARDUINO_WIFI_REALTIME_SWITCH_MEMORYMONITOR_H

#include <MemoryUsage.h>
#include "../constants/General.h"
#include "TaskScheduler.h"


class MemoryMonitor {
private:
    TaskScheduler scheduler;

    static const void printRam() {
        Serial.println(F("Starting state of the memory:"));
        Serial.println();

        MEMORY_PRINT_START
                MEMORY_PRINT_HEAPSTART
        MEMORY_PRINT_HEAPEND
                MEMORY_PRINT_STACKSTART
        MEMORY_PRINT_END
                MEMORY_PRINT_HEAPSIZE

        Serial.println();


        FREERAM_PRINT;

        Serial.println();
        Serial.println();
    }

public:
    void init() {
        scheduler.init(General::MEMORY_MONITOR_INTERVAL_MILLIS, true, []() {
            printRam();
        });

        printRam();
    }

    void tick() {
        this->scheduler.tick();
    }
};

#endif //ARDUINO_WIFI_REALTIME_SWITCH_MEMORYMONITOR_H
