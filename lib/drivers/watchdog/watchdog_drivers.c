#include "watchdog_drivers.h"

IWDG_HandleTypeDef hiwdg;

void Watchdog_Init(){
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 2500;
}