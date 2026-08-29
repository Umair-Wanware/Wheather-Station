#pragma once
#include "stdint.h"

struct DiagnosticsData{
    bool wifiConnected;
    uint32_t freeHeap, minimumHeap, idleCounter;
    uint32_t networkStack, parserStack, displayStack; 
    uint32_t watchdogStack;
};