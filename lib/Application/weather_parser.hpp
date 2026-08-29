#pragma once
#include <stdbool.h>

struct WeatherData {
    float temperature, humidity, pressure, windSpeed;
    char condition[32], city[32];
};

class WeatherParser {
    public:
    static bool Parse(const char* response, WeatherData& data);

    private:
    static const char* FindJson(const char* response);
};