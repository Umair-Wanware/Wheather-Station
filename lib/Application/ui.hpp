#pragma once
#include "weather_parser.hpp"
#include "diagnostics.hpp"

enum class DisplayPage {
    WEATHER,
    ENVIROMENT,
    WIND,
    SYSTEM,
    DIAGNOSTICS,

    COUNT
};

class UI {
    public:
    static void Init();
    static void Draw(const WeatherData& weather, const DiagnosticsData& dignostics);
    static void NextPage();

    private:
    static DisplayPage currentPage;

    static void DrawWeather(const WeatherData&);
    static void DrawEnviroment(const WeatherData&);
    static void DrawWind(const WeatherData&);
    static void DrawSystem(const WeatherData&);
    static void DrawDiagnostics(const DiagnosticsData&);
};