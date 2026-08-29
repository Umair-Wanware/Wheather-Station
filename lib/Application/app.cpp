#include "app.hpp"
#include "weather_parser.hpp"
#include "uart/uart_drivers.h"
#include "esp8266/esp8266.hpp"
#include "ssd1306/ssd1306.h"
#include "ssd1306/ssd1306_fonts.h"
#include "i2c/i2c_drivers.h"
#include "packet.hpp"
#include "ui.hpp"
#include "watchdog.hpp"
#include "diagnostics.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

TaskHandle_t networkHandle, parserHandle = nullptr;
TaskHandle_t displayHandle, watchdogHandle, diagnosticsHandle = nullptr;

volatile uint32_t idleCounter = 0;
extern "C" {
    void vApplicationIdleHook(){
        idleCounter++;
        __WFI();
    }

    void vApplicationMallocFailedHook(){
        taskDISABLE_INTERRUPTS();
        while(1);
    }

    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName){
        (void)xTask;
        (void)pcTaskName;

        taskDISABLE_INTERRUPTS();
        while(1);
    }
}

class NetworkTask {
    public:
    NetworkTask(QueueHandle_t nQ) : networkQueue(nQ) {};
    void Start(){
        xTaskCreate(task_entry, "NETWORK", 1024, this, 5, &networkHandle);
    }

    private:
    QueueHandle_t networkQueue;
    ESP8266 wifi;

    static void task_entry(void *pvPara){
        static_cast<NetworkTask*>(pvPara)->Run();
        vTaskDelete(nullptr);
    }
    void Run(){
        HttpPacket packet;

        constexpr const char* ssid = "Umair's Galaxy";
        constexpr const char* password = "umair@00";

        constexpr const char* host = "api.open-meteo.com";
        constexpr const char* path = "/v1/forecast?" "latitude=18.5204&" "longitude=73.8567&"
                                     "current=temperature_2m," "relative_humidity_2m," "pressure_msl," "wind_speed_10m";

        wifi.Init();
        wifi.ConnectWifi(ssid, password);

        TickType_t lastWakeTime = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(600000);

        while(true){
            if(wifi.HTTPGet(host, path, packet.response, sizeof(packet.response)) == true){
                xQueueOverwrite(networkQueue, &packet);
            }
            Watchdog::Kick(WATCHDOG_NETWORK);
            vTaskDelayUntil(&lastWakeTime, period);
        }
    }
};

class ParserTask {
    public:
    ParserTask(QueueHandle_t nQ, QueueHandle_t wQ) : networkQueue(nQ), weatherQueue(wQ) {};
    void Start(){
        xTaskCreate(task_entry, "PARSE", 1024, this, 4, &parserHandle);
    }

    private:
    QueueHandle_t networkQueue, weatherQueue;
    static void task_entry(void *pvPara){
        static_cast<ParserTask*>(pvPara)->Run();
        vTaskDelete(nullptr);
    }

    void Run(){
        HttpPacket packet;
        WeatherData weather;

        while(true){
            if(xQueueReceive(networkQueue, &packet, portMAX_DELAY) == pdPASS){
                if(WeatherParser::Parse(packet.response, weather) == true){
                    xQueueOverwrite(weatherQueue, &weather);
                }
            }
            Watchdog::Kick(WATCHDOG_PARSER);
        }
    }
};

class DisplayTask {
    public:
    DisplayTask(QueueHandle_t wQ, QueueHandle_t dQ) : weatherQueue(wQ), diagnosticsQueue(dQ) {};
    void Start(){
        xTaskCreate(task_entry, "DISPLAY", 2048, this, 3, &displayHandle);
    }

    private:
    QueueHandle_t weatherQueue, diagnosticsQueue;
    static void task_entry(void *pvPara){
        static_cast<DisplayTask*>(pvPara)->Run();
        vTaskDelete(nullptr);
    }

    void Run(){
        WeatherData weather;
        DiagnosticsData diagnostics;
        UI::Init();

        TickType_t lastWakeTime = xTaskGetTickCount();
        constexpr TickType_t period = pdMS_TO_TICKS(250);
        TickType_t pageTimer = xTaskGetTickCount();

        while(true){
            if(xQueuePeek(weatherQueue, &weather, 0) == pdPASS){
                if(xQueuePeek(diagnosticsQueue, &diagnostics, 0) == pdPASS){
                    if((xTaskGetTickCount() - pageTimer) >= pdMS_TO_TICKS(3000)){
                        UI::NextPage();
                        pageTimer = xTaskGetTickCount();
                    }
                    UI::Draw(weather, diagnostics);
                    Watchdog::Kick(WATCHDOG_DISPLAY);
                    vTaskDelayUntil(&lastWakeTime, period);
                }
            }
        }
    }
};

class WatchdogTask {
    public:
    void Start(){
        xTaskCreate(task_entry, "WATCHDOG", 512, this, 2, &watchdogHandle);
    }

    private:
    static void task_entry(void *pvPara){
        static_cast<WatchdogTask*>(pvPara)->Run();
        vTaskDelete(nullptr);
    }
    void Run(){
        TickType_t lastWakeTime = xTaskGetTickCount();
        constexpr TickType_t period = pdMS_TO_TICKS(1000);
        constexpr uint8_t expected = WATCHDOG_NETWORK | WATCHDOG_PARSER | WATCHDOG_DISPLAY;

        Watchdog watch;

        while(true){
            if(watch.AllTasksAlive(expected)){
                watch.ResetHeartbeats();
                watch.Feed();
                vTaskDelayUntil(&lastWakeTime, period);
            }
        }
    }
};

class DiagnosticsTask {
    public:
    DiagnosticsTask(QueueHandle_t dQ) : diagnosticsQueue(dQ) {};
    void Start(){
        xTaskCreate(task_entry, "DIAGNOSTICS", 1024, this, 1, &diagnosticsHandle);
    }

    private:
    QueueHandle_t diagnosticsQueue;
    ESP8266 esp;
    static void task_entry(void *pvPara){
        static_cast<DiagnosticsTask*>(pvPara)->Run();
        vTaskDelete(nullptr);
    }

    void Run(){
        DiagnosticsData data;

        TickType_t lastWakeTime = xTaskGetTickCount();
        constexpr TickType_t period = pdMS_TO_TICKS(1000);

        while(true){
            data.freeHeap = xPortGetFreeHeapSize();
            data.minimumHeap = xPortGetMinimumEverFreeHeapSize();
            data.idleCounter = idleCounter;
            data.wifiConnected = esp.IsConnected();
            data.networkStack = uxTaskGetStackHighWaterMark(networkHandle);
            data.parserStack = uxTaskGetStackHighWaterMark(parserHandle);
            data.displayStack = uxTaskGetStackHighWaterMark(displayHandle);
            data.watchdogStack = uxTaskGetStackHighWaterMark(watchdogHandle);

            xQueueOverwrite(diagnosticsQueue, &data);
            vTaskDelayUntil(&lastWakeTime, period);
        }
    }
};

void App_Init(){
    UART_Init();
    I2C_Init();
    ssd1306_Init();
    Watchdog::Init();

    QueueHandle_t networkQueue = xQueueCreate(1, sizeof(HttpPacket));
    QueueHandle_t weatherQueue = xQueueCreate(1, sizeof(WeatherData));
    QueueHandle_t diagnosticsQueue = xQueueCreate(1, sizeof(DiagnosticsData));
    configASSERT(networkQueue);
    configASSERT(weatherQueue);
    configASSERT(diagnosticsQueue);

    static NetworkTask networkTask(networkQueue);
    static ParserTask parserTask(networkQueue, weatherQueue);
    static DisplayTask displayTask(weatherQueue, diagnosticsQueue);
    static WatchdogTask watchTask;
    static DiagnosticsTask diagnosticsTask(diagnosticsQueue);

    networkTask.Start();
    parserTask.Start();
    displayTask.Start();
    watchTask.Start();
    diagnosticsTask.Start();

    vTaskStartScheduler();

    while(true);
}