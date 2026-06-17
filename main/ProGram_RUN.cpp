#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"
#include <iostream>
#include <cmath> 

void Stuck(int n, float coordlatitude, float coordlongtitude)//nはスタック解消の試行回数事前設定必要
{
    float before_stuckLat = coordlatitude;
    float before_stuckLon = coordlongtitude;

    sendTelemetryText("スタック解消動作を行います");

     gpio_set_level(GPIO_NUM_25,1);
     gpio_set_level(GPIO_NUM_33,1); //反時計回り超信地旋回
    sendTelemetryText("反時計回り超信地旋回を行います");
     vTaskDelay(pdMS_TO_TICKS(2000));

    sendTelemetryText("回転を停止します");
     gpio_set_level(GPIO_NUM_25,0); //モータ停止
     gpio_set_level(GPIO_NUM_33,0); //モータ停止

    sendTelemetryText("時計回り超信地旋回を行います");
     gpio_set_level(GPIO_NUM_26,1);
     gpio_set_level(GPIO_NUM_32,1); 
     vTaskDelay(pdMS_TO_TICKS(1000));
    
    sendTelemetryText("回転を停止します");
     gpio_set_level(GPIO_NUM_26,0); //モータ停止
     gpio_set_level(GPIO_NUM_32,0); //モータ停止

    sendTelemetryText("前進します");
     gpio_set_level(GPIO_NUM_25,1);
     gpio_set_level(GPIO_NUM_26,1);
     vTaskDelay(pdMS_TO_TICKS(4000));

    sendTelemetryText("モータを停止します");
     gpio_set_level(GPIO_NUM_25,0); //モータ停止
     gpio_set_level(GPIO_NUM_26,0); //モータ停止
    
    taskENTER_CRITICAL(&gps_mux);
    float after_stuckLat = g_coordlatitude;
    float after_stuckLon = g_coordlongtitude;
    taskEXIT_CRITICAL(&gps_mux);
    float distance_NowANDThenLon = std::abs(after_stuckLat - before_stuckLat);
    float distance_NowANDthenLat = std::abs(after_stuckLon - before_stuckLon);
    float DistanceStuckABS = sqrt(distance_NowANDthenLat*distance_NowANDthenLat 
        + distance_NowANDThenLon*distance_NowANDThenLon);
    
    if(DistanceStuckABS < 0.5 && n >= 1)
    {
        n = n - 1;
        char msg[64];
        snprintf(msg, sizeof(msg), "残りスタック解消動作実行回数: %d", n);
        sendTelemetryText("スタックが解消できていないようです…引き続きスタック処理を行います");
        sendTelemetryText(msg);
        Stuck(n, g_coordlatitude, g_coordlongtitude);
        return;
    }
    else if(DistanceStuckABS < 0.5 && n <= 0)
    {
        sendTelemetryText("スタックが解消できませんでした。機体位置を手動で解決してください");
        sendTelemetryText("スタック解消できませんでした。スタック解消プログラムを終了します");
        return;
    }
    else
    {
        sendTelemetryText("スタック解消完了、スタック解消プログラムを終了します");
        return;
    }
}

struct Vec2 { double x, y;};
struct Distination   { float x, y; };
struct Now_Body_vector { float x, y; };
struct Now_Body_vector body = { float angels.yaw,angels.pitch;};
const Distination Target = {本番用の目的地緯度,本番用の目的地経度};

void Cul_NAVVec(const Vec2 Target, const Vec2 Here)
{
    Vec2 Target = Distination Target;
    Vec2 Here = {coord.latitude,coord.longtitude};

    //二点間のベクトルを算出
    Vec2 Nav = 
    {
      target.x - here.x,;
      target.y - here.y;
    };
    //正規化
    float norm =std::sqrt(Nav.x*Nav.x + Nav.y*Nav.y);
    Nav = {Nav.x/norm, Nav.y/norm};
}

void LotateCul(float Nav_x, float Nav_y)
{
    double dot = Nav_x * body_x + Nav_y * body_y;
    double cross = Nav_x * body_y - Nav_y * body_x;
    double theta = std::atan2(cross,dot);
    double lotate_degree = theta * 180.0 / std::acos(-1.0);
    sendTelemetryText(std::format("機体回転角算出 = {}",degree));
}

void loop_Make_Body_lotate()
{
    double lotate_number = angels.yow + lotate_degree;
    if(lotate_number <= 2)
    {
        gpio_set_level(GPIO_NUM_25,0); //モータ停止
        gpio_set_level(GPIO_NUM_26,0); //モータ停止
        gpio_set_level(GPIO_NUM_32,0); //モータ停止
        gpio_set_level(GPIO_NUM_33,0); //モータ停止
        sendTelemetryText("機体方向補正回転停止");
        return;
    }
    else
    {
        if (lotate_number >= 0)
        {
            gpio_set_level(GPIO_NUM_25,1);
            gpio_set_level(GPIO_NUM_33,1); //反時計回り超信地旋回
            sendTelemetryText("右回転");
        }
        else if (lotate_number < 0)
        (
            gpio_set_level(GPIO_NUM_26,1);
            gpio_set_level(GPIO_NUM_32,1); //時計回り超信地旋回
            sendTelemetryText("左回転");
        )
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void GPS_STANBY(int retry = 試行回数)
{
    if(retry <= 0)
    {
        sendTelemetryText("位置取得失敗");
        return;
    }

    GpsCoordinate coord = {};
    getGpsCoordinate(GpsCoordinate &coord);
    float now_lat = Target.x;
    float now_lon = Target.y;
    
    if(std::abs(coord.latitude-Target.x)< 任意の誤差範囲 &&
       std::abs(coord.longtitude-Target.y)< 任意の誤差範囲)
    {
        sendTelemetryText("GPS数値取得成功");
        return;
    }
    else
    {
        sendTelemetryText("GPS数値取得失敗");
        GPS_STANBY(retry - 1);
    }
}

void NavVector()
{
    Cul_NAVVec(Target, Here);
    LotateCul(Nav_x, Nav_y);
    loop_Make_Body_lotate();
    
}

loop_main_RUN()
{

//空いた駅を少し走る
}

extern "C" void app_main_RUN()
{

}