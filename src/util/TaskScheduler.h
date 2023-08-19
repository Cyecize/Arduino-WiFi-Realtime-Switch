#ifndef ARDUINO_WIFI_REALTIME_SWITCH_TASKSCHEDULER_H
#define ARDUINO_WIFI_REALTIME_SWITCH_TASKSCHEDULER_H

#include <Arduino.h>

typedef void (*ScheduleCallback)();

class TaskScheduler {
private:
    unsigned long startMillis;
    long interval;
    ScheduleCallback callback;
    bool running = false;
    bool loop = false;

public:
    void init(long intervalMillis, bool isLoop, const ScheduleCallback &callbackFunc) {
        this->interval = intervalMillis;
        this->callback = callbackFunc;
        this->running = true;
        this->loop = isLoop;
    }

    void reset() {
        this->startMillis = millis();
    }

    void tick() {
        if (!this->running) {
            return;
        }

        unsigned long now = millis();

        // In case millis has overflown after 49 days, prevent endless cycle with that.
        if (this->startMillis > now) {
            this->startMillis = now;
        }

        if (now - this->startMillis > this->interval) {
            this->callback();
            this->startMillis = now;

            if (!this->loop) {
                this->running = false;
            }
        }
    }
};

#endif //ARDUINO_WIFI_REALTIME_SWITCH_TASKSCHEDULER_H
