//このプログラムは待機状態でのテレメトリを担当する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"

void loop_STANBY()
{
    gpio_set_direction((gpio_num_t)RXD_PIN_S3sense, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)TXD_PIN_S3sense, GPIO_MODE_OUTPUT);

    int pinState = gpio_get_level((gpio_num_t)RXD_PIN_S3sense);

    if(pinState == 1)
    {
        for (int i = 0; i < 10; i++)
        {
            char msg[64];
            snprintf(msg, sizeof(msg), "画像認識検知成功: %d回", i);
            sendTelemetryText(msg);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        sendTelemetryText("画像認識検知成功10回");
        sendTelemetryText("画像認識をスタンバイモードに移行します");
        gpio_set_level(TXD_PIN_S3sense, 1);
        sendTelemetryText("FRYINGに移行します");
        currentState = CanSatState::FRYING;
        return;
    }
    else
    {
        sendTelemetryText("画像認識検知なし");
        sendTelemetryText("ROG準備中待機");
    }
}
