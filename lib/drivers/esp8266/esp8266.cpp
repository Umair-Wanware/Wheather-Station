#include "esp8266.hpp"
#include "uart/uart_drivers.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>

bool ESP8266::Init(){
    if(!SendCommand("AT\r\n", "OK", 1000)) return false;
    if(!SendCommand("ATE0\r\n", "OK", 1000)) return false;
    if(!SendCommand("AT+CWMODE=1\r\n", "OK", 1000)) return false;

    return true;
}

bool ESP8266::ConnectWifi(const char* ssid, const char* password){
    char command[128];

    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\", \"%s\"\r\n", ssid, password);
    bool success = SendCommand(command, "WIFI GOT IP", 20000);

    connected = success;
    return connected;
}

bool ESP8266::HTTPGet(const char* host, const char* path, char* response, uint32_t responseSize){
    char command[128];

    snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\", \"%s","80\r\n");

    if(!SendCommand(command, "OK", 5000)) return false;

    char httpRequest[512];

    snprintf(httpRequest, sizeof(httpRequest), "GET %s HTTP/1.1\r\n" "Host: %s\r\n" "Connection: close\r\n\r\n", path, host);
    sniprintf(command, sizeof(command), "AT+CIPSEND=%d\r\n", (int)strlen(httpRequest));

    if(!SendCommand(command, ">", 3000)) return false;

    HAL_UART_Transmit(&huart, reinterpret_cast<uint8_t*>(httpRequest), strlen(httpRequest), HAL_MAX_DELAY);

    if(!Receive(response, responseSize, 10000)) return false;
}

bool ESP8266::SendCommand(const char* cmd, const char* expected, uint32_t timeout){
    HAL_UART_Transmit(&huart, reinterpret_cast<uint8_t*>(const_cast<char*>(cmd)), strlen(cmd), HAL_MAX_DELAY);

    char buffer[512] = {0};
    
    if(!Receive(buffer, sizeof(buffer), timeout)) return false;

    return strstr(buffer, expected) != nullptr;
}

bool ESP8266::Receive(char* buffer, uint32_t length, uint32_t timeout){
    uint32_t start = HAL_GetTick();
    uint32_t index = 0;

    while((HAL_GetTick() - start) < timeout){
        uint8_t ch;
        if(HAL_UART_Receive(&huart, &ch, 1, 10) == HAL_OK){
            if(index < length - 1){
                buffer[index++] = static_cast<char>(ch);
            }
        }
    }
    buffer[index] = '\0';
    return index > 0;
}

bool ESP8266::IsConnected() const {
    return connected;
}