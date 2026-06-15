//このプログラムは待機状態でのテレメトリを担当する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"

void loop_STANBY()
{
    gpio_set_direction((gpio_num_t)RXD_PIN_Zero2W, GPIO_MODE_INPUT);

    int pinState = gpio_get_level((gpio_num_t)RXD_PIN_Zero2W);

    if(pinState == 1)
    {
        for (int i = 0; i < 10; i++)
        {
            sendTelemetryText("画像認識検知成功");
        }
        currentState = CanSatState::LANDING;
        return;
    }
    else
    {
        sendTelemetryText("画像認識検知なし");
        sendTelemetryText("ROG準備中待機");
    }
}

void STANBY_main()
{
    loop_STANBY();
    return;
}