#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"
#include <iostream>
#include <cmath> 

void Stuck()
{

}
struct Vec2 
{
    double x;
    double y;
};

struct Distination
{ 
    float x;
    float y;
};

struct Now_Body_vector
{
    float x;
    float y;
};

Now_Body_vector body =
{
    float angels.yaw
    float angels.
};

const Distination Target = {本番用の目的地緯度,本番用の目的地経度};

void Cul_NAVVec(const Vec2 Target, const Vec2 Here)
{
    Vec2 Target = Distination Target;
    Vec2 Here = {coord.latitude,coord.longtitude};

    //二点間のベクトルを算出
    Vec2 Nav = 
    {
      target.x - here.x,
      target.y - here.y
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
        sendTelemetryText("機体方向補正回転停止")
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
    getGpsCoordinate(GpsCoordinate &coord)
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


}

extern "C" void app_main_RUN()
{

}