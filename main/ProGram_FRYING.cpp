//このプログラムは機体の動作開始を意味する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

void setup_FRYING()
{
    gpio_set_direction((gpio_num_t)FALLOUT_PIN_BLUP, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)FALLOUT_PIN_GND, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode((gpio_num_t)FALLOUT_PIN_BLUP, GPIO_PULLUP_ONLY);
}

void loop_FRYING()
{
    int fallpinState = gpio_get_level((gpio_num_t)FALLOUT_PIN_BLUP);

    if(fallpinState == 1)
    {
        int64_t startTime = esp_timer_get_time();
        sendTelemetryText("ピン抜け検知");

        while((esp_timer_get_time() - startTime) < 2000000 && (g_accel.z < 30.0))
        {
            updateAccel();
            char ACCEL[64];
            snprintf(ACCEL, sizeof(ACCEL), "Accel: %.2f, %.2f, %.2f m/s^2\n"
            , g_accel.x, g_accel.y, g_accel.z);
            sendTelemetryText(ACCEL);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if((esp_timer_get_time() - startTime) >= 2000000)
        {
            sendTelemetryText("タイムアウトよりSTATEをSTARTに強制移行します");
            currentState = CanSatState::START;
        }

        else if((esp_timer_get_time() - startTime) < 2000000 && (g_accel.z >= 30.0) )
        {
            sendTelemetryText("落下衝撃を検知しました");
            vTaskDelay(pdMS_TO_TICKS(5000));
            sendTelemetryText("STATEをSTARTに移行します");
            currentState = CanSatState::START;
        }
        return;
    }   
    vTaskDelay(pdMS_TO_TICKS(100));
}