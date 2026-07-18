//このプログラムはCanSatが機体に着地した時に展開する操作を担当している

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>

extern float getYaw(); // 九軸センサー専用コード.cpp（現状CMakeLists未登録、別途要修正）

float g_bodyEast   = 0.0f;
float g_bodyNorth  = 0.0f;
float g_prevYaw    = 0.0f;
float g_yawOffset  = 0.0f;
bool  g_calibrated = false;

void setup_START()
{
    gpio_set_direction((gpio_num_t)PIN_NICROM, GPIO_MODE_OUTPUT);

    gpio_set_direction((gpio_num_t)PIN_RMOTOR_FRONT, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_RMOTOR_BACK, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_LMOTOR_FRONT, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_LMOTOR_BACK, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_RMOTOR_FRONT, 0);
    gpio_set_level((gpio_num_t)PIN_RMOTOR_BACK, 0);
    gpio_set_level((gpio_num_t)PIN_LMOTOR_FRONT, 0);
    gpio_set_level((gpio_num_t)PIN_LMOTOR_BACK, 0);

    // 機体前方ベクトル（ワールド座標・ENU）
    g_bodyEast  = 0.0f;
    g_bodyNorth = 0.0f;
    g_prevYaw   = 0.0f;  // 前回のIMU Yaw
}

void calibrateBodyVector(int n) 
{
    vTaskDelay(pdMS_TO_TICKS(100));

    taskENTER_CRITICAL(&gps_mux);
    float startLat = g_coordlatitude;
    float startLon = g_coordlongtitude;
    taskEXIT_CRITICAL(&gps_mux);


    sendTelemetryText("前進します");
    gpio_set_level((gpio_num_t)PIN_RMOTOR_FRONT, 1);
    gpio_set_level((gpio_num_t)PIN_LMOTOR_FRONT, 1); // 絶対方位測定前進
    vTaskDelay(pdMS_TO_TICKS(5000));
    gpio_set_level((gpio_num_t)PIN_RMOTOR_FRONT, 0); 
    gpio_set_level((gpio_num_t)PIN_LMOTOR_FRONT, 0);

    taskENTER_CRITICAL(&gps_mux);
    float endLat = g_coordlatitude;
    float endLon = g_coordlongtitude;
    taskEXIT_CRITICAL(&gps_mux);

    float dNorth = endLat - startLat;
    float dEast  = endLon - startLon;

    if(sqrt(dNorth*dNorth + dEast*dEast) < 0.00005f) 
    {
        sendTelemetryText("スタックしています");
        if (n <= 0)
        {
            sendTelemetryText("スタック解消不可");
            sendTelemetryText("プログラムを終了します");
            return;
        }
        else
        {
            n = n-1;
            sendTelemetryText("スタック解消動作を行います");
            Stuck(n, g_coordlatitude, g_coordlongtitude);
            vTaskDelay(pdMS_TO_TICKS(100));
            return;
        }
    }
    // GPS差分から絶対方位を計算
    float gpsBearing = atan2(dEast, dNorth) * 180.0f / M_PI;
    g_yawOffset = gpsBearing - getYaw();

    // この時点の機体ベクトルを確定
    float rad = gpsBearing * M_PI / 180.0f;
    g_bodyEast  = sin(rad);
    g_bodyNorth = cos(rad);

    g_prevYaw   = getYaw();
    g_calibrated = true;

    char BodyVector[64];
    snprintf(BodyVector, sizeof(BodyVector), "GPS: %.2f, bodyEast: %.2f, bodyNort: %.2f \n"
            ,gpsBearing, g_bodyEast, g_bodyNorth);
    sendTelemetryText(BodyVector);
}

void loop_START()
{
    int n = 10; //スタック解消の最大試行回数
    sendTelemetryText("ニクロム線起動開始");
    gpio_set_level((gpio_num_t)PIN_NICROM, 1);
    vTaskDelay(pdMS_TO_TICKS(1500));
    gpio_set_level((gpio_num_t)PIN_NICROM, 0);
    sendTelemetryText("ニクロム線起動完了");
    sendTelemetryText("これより機体ベクトル算出動作を行います");
    calibrateBodyVector(n);

    sendTelemetryText("STATEをRUNに移行します");
    currentState = CanSatState::RUN;
    return;
}
