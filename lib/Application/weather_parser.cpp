#include "weather_parser.hpp"
#include "cJSON.h"
#include <string.h>

const char* WeatherParser::FindJson(const char* response){
    if(response == nullptr){
        return nullptr;
    }

    const char* json = strstr(response, "\r\n\r\n");

    if(json == nullptr){
        return nullptr;
    }
    return json + 4;
}

bool WeatherParser::Parse(const char* response, WeatherData& data){
    memset(&data, 0, sizeof(WeatherData));
    const char* json = FindJson(response);

    if(json == nullptr){
        return false;
    }
    cJSON* root = cJSON_Parse(json);

    if(root == nullptr){
        return false;
    }

    cJSON* current = cJSON_GetObjectItem(root, "current");

    if(current == nullptr){
        cJSON_Delete(root);
        return false;
    }

    cJSON* temperature = cJSON_GetObjectItem(current, "temperature_2m");
    cJSON* humidity = cJSON_GetObjectItem(current, "relative_humidity_2m");
    cJSON* pressure = cJSON_GetObjectItem(current, "pressure_msl");
    cJSON* wind = cJSON_GetObjectItem(current, "wind_speed_10m");

    if(temperature){
        data.temperature = temperature->valuedouble;
    }
    if(humidity){
        data.humidity = humidity->valuedouble;
    }
    if(pressure){
        data.pressure = pressure->valuedouble;
    }
    if(wind){
        data.windSpeed = wind->valuedouble;
    }

    cJSON* timezone = cJSON_GetObjectItem(root, "timezone");

    if(timezone != nullptr){
        strncpy(data.city, timezone->valuestring, sizeof(data.city) - 1);
    }
    strcpy(data.condition, "Unknown");
    
    cJSON_Delete(root);

    return true;
}