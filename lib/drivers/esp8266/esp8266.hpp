#pragma once

#include <stdint.h>
#include <stdbool.h>

class ESP8266 {
    public:
    static bool Init();
    bool ConnectWifi(const char* ssid, const char* password);
    static bool HTTPGet(const char* host, const char* path, char* response, uint32_t responseSize);
    bool IsConnected() const;

    private:
    static bool SendCommand(const char* cmd, const char* expected, uint32_t timeout);
    static bool Receive(char* buffer, uint32_t length, uint32_t timeout);
    bool connected;
};