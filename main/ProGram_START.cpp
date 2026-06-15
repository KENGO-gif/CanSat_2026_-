//このプログラムはCanSatが機体に着地した時に展開する操作を担当している

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void setup_START()
{
    gpio_set_direction((gpio_num_t)PIN_NICROM, GPIO_MODE_OUTPUT);

    gpio_set_level((gpio_num_t)PIN_NICROM, 0);
}

// 機体前方ベクトル（ワールド座標・ENU）
float bodyEast  = 0.0f;
float bodyNorth = 0.0f;

float prevYaw = 0.0f;  // 前回のIMU Yaw

void calibrateBodyVector(int n, float coordlatitude, float coordlongtitude) 
{
    vTaskDelay(pdMS_TO_TICKS(100));
    float startLat = coordlatitude;
    float startLan = coordlongtitude;

    gpio_set_level(GPIO_NUM_25,1);
    gpio_set_level(GPIO_NUM_32,1); // 絶対包囲測定全身

    float dNorth = coordlatitude - startLat;
    float dEast  = coordlongtitude - startLon;

    if(sqrt(dNorth*dNorth + dEast*dEast) < 0.00005f) 
    {
        sendTelemetryText("スタックしています");
        if (n <= 0)
        {
            sendTelemetryText("スタック解消不可");
            sendTelemetryText("終了します");
            return;
        }
        else
        {
            n = n-1;
            sendTelemetryText("スタック解消動作");
            stuck();
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        }
    }
    // GPS差分から絶対方位を計算
    float gpsBearing = atan2(dEast, dNorth) * 180.0f / PI;
    yawOffset = gpsBearing - getYaw();

    // この時点の機体ベクトルを確定
    float rad = gpsBearing * PI / 180.0f;
    bodyEast  = sin(rad);
    bodyNorth = cos(rad);

    prevYaw   = getYaw();
    calibrated = true;

    sendTelemetryText("%.2f %.3f %.3f\n",gpsBearing, bodyEast, bodyNorth);

}

void loop_START()
{
    sendTelemetryText("ニクロム線起動開始");

    gpio_set_level((gpio_num_t)PIN_NICROM, 1);

    vTaskDelay(pdMS_TO_TICKS(1000));

    gpio_set_level((gpio_num_t)PIN_NICROM, 0);

    sendTelemetryText("ニクロム線起動完了");
    sendTelemetryText("GOALに移行します");

    currentState = CanSatState::RUN;

    return;
}
