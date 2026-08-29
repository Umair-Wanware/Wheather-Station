#include "watchdog.hpp"
#include "stm32f1xx_hal.h"
#include "watchdog/watchdog_drivers.h"

volatile uint8_t Watchdog::heartbeat = 0;
IWDG_HandleTypeDef hiwdg;

void Watchdog::Init(){
    heartbeat = 0;
}

void Watchdog::Kick(WatchdogTaskFlag task){
    heartbeat |= task;
}

void Watchdog::Feed(){
    HAL_IWDG_Refresh(&hiwdg);
}

void Watchdog::ResetHeartbeats(){
    heartbeat = 0;
}

bool Watchdog::AllTasksAlive(uint8_t expected){
    return heartbeat == expected;
}