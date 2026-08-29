#ifndef WATCHDOG_DRIVERS_H
#define WATCHDOG_DRIVERS_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
    #endif

    void Watchdog_Init();
    void Watchdog_Refresh();

    #ifdef __cplusplus
}
#endif
#endif