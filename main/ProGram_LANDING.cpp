//このプログラムは機体の動作開始を意味する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void setup_LANDING()
{
    gpio_set_direction((gpio_num_t)FALLOUT_PIN_BLUP, GPIO_MODE_OUTPUT);

    gpio_set_direction((gpio_num_t)FALLOUT_PIN_GND, GPIO_MODE_INPUT);

    gpio_set_level((gpio_num_t)FALLOUT_PIN_BLUP, 1);
}

void loop_LANDING()
{
    int fallpinState = gpio_get_level((gpio_num_t)FALLOUT_PIN_GND);
    
    if(fallpinState == 0)
    {
        sendTelemetryText("ピン抜け検知");
        sendTelemetryText("LANDINGに移行します");

        currentState = CanSatState::START;
        return;
    }   
    else
    {
        sendTelemetryText("ピンはまだ接続されています");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}