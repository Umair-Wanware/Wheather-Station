#include "ui.hpp"
#include "ssd1306/ssd1306.h"
#include "ssd1306/ssd1306_fonts.h"
#include "diagnostics.hpp"
#include <stdio.h>

DisplayPage UI::currentPage = DisplayPage::WEATHER;

void UI::Init(){
    currentPage = DisplayPage::WEATHER;
}

void UI::Draw(const WeatherData& weather, const DiagnosticsData& diagnostics){
    ssd1306_Fill(Black);

    switch(currentPage){
        case DisplayPage::WEATHER:
            DrawWeather(weather);
            break;
        case DisplayPage::ENVIROMENT:
            DrawEnviroment(weather);
            break;
        case DisplayPage::WIND:
            DrawWind(weather);
            break;
        case DisplayPage::SYSTEM:
            DrawSystem(weather);
            break;
        case DisplayPage::DIAGNOSTICS:
            DrawDiagnostics(diagnostics);
            break;
        default:
            break;
    }
    ssd1306_UpdateScreen();
}

void UI::NextPage(){
    currentPage = static_cast<DisplayPage>((static_cast<int>(currentPage) + 1) % static_cast<int>(DisplayPage::COUNT));
}

void UI::DrawWeather(const WeatherData& weather){
    char text[32];

    ssd1306_SetCursor(0, 0);
    snprintf(text, sizeof(text), "Weather");
    ssd1306_WriteString(text, Font_11x18, White);

    snprintf(text, sizeof(text), "%.1f C", weather.temperature);

    ssd1306_SetCursor(0, 28);
    ssd1306_WriteString(text, Font_16x26, White);
}

void UI::DrawEnviroment(const WeatherData& weather){
    char text[32];

    ssd1306_SetCursor(0, 0);
    snprintf(text, sizeof(text), "Enviroment");
    ssd1306_WriteString(text, Font_7x10, White);

    snprintf(text, sizeof(text), "Humidity %.1f%%", weather.humidity);

    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString(text, Font_7x10, White);

    snprintf(text, sizeof(text), "Presure %.1f", weather.pressure);

    ssd1306_SetCursor(0, 40);
    ssd1306_WriteString(text, Font_7x10, White);
}

void UI::DrawWind(const WeatherData& weather){
    char text[32];

    ssd1306_SetCursor(0, 0);
    snprintf(text, sizeof(text), "Wind");
    ssd1306_WriteString(text, Font_11x18, White);

    snprintf(text, sizeof(text), "%.1f km/h", weather.windSpeed);
    ssd1306_SetCursor(0, 30);
    ssd1306_WriteString(text, Font_11x18, White);
}

void UI::DrawSystem(const WeatherData& weather){
    (void)weather;
    char text[32];

    ssd1306_SetCursor(0, 0);
    snprintf(text, sizeof(text), "System");
    ssd1306_WriteString(text, Font_11x18, White);

    ssd1306_SetCursor(0, 28);
    snprintf(text, sizeof(text), "WIFI: Connected");
    ssd1306_WriteString(text, Font_11x18, White);

    ssd1306_SetCursor(0, 40);
    snprintf(text, sizeof(text), "Weather: Ok");
    ssd1306_WriteString(text, Font_11x18, White);
}

void UI::DrawDiagnostics(const DiagnosticsData& diagnostics){
    char text[32];

    ssd1306_SetCursor(0, 0);
    snprintf(text, sizeof(text), "Diagnostics");
    ssd1306_WriteString(text, Font_7x10, White);

    ssd1306_SetCursor(0, 16);
    snprintf(text, sizeof(text), "Heap: %lu", diagnostics.freeHeap);
    ssd1306_WriteString(text, Font_7x10, White);

    ssd1306_SetCursor(0, 28);
    snprintf(text, sizeof(text), "Idle: %lu", diagnostics.idleCounter);
    ssd1306_WriteString(text, Font_7x10, White);

    ssd1306_SetCursor(0, 40);
    if(diagnostics.wifiConnected){
        snprintf(text, sizeof(text), "Wifi Ok");
        ssd1306_WriteString(text, Font_7x10, White);
    } else {
        snprintf(text, sizeof(text), "Wifi Lost");
        ssd1306_WriteString(text, Font_7x10, White);
    }
}