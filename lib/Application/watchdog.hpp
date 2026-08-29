#pragma once
#include <stdint.h>

enum WatchdogTaskFlag : uint8_t {
    WATCHDOG_NETWORK = (1 << 0),
    WATCHDOG_PARSER  = (1 << 1), 
    WATCHDOG_DISPLAY = (1 << 2)
};

class Watchdog {
    public:
    static void Init();
    static void Feed();
    static void Kick(WatchdogTaskFlag task);
    bool AllTasksAlive(uint8_t expected);
    static void ResetHeartbeats();

    private:
    static volatile uint8_t heartbeat;
};