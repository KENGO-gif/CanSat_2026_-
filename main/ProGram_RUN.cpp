#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "driver/gpio.h"
#include <iostream>
#include <cstdio>
#include <cmath>

extern float getYaw();     // 九軸センサー専用コード.cpp
extern float g_bodyEast;   // ProGram_START.cpp（機体前方ベクトル・東成分）
extern float g_bodyNorth;  // ProGram_START.cpp（機体前方ベクトル・北成分）

struct Vec2 { double x, y; };
struct Distination { float x, y; };

constexpr float  DEST_LAT = 0.0f;                   // 要設定：本番の目的地緯度
constexpr float  DEST_LON = 0.0f;                   // 要設定：本番の目的地経度
constexpr int    GPS_STANBY_MAX_RETRY = 5;          // 要調整：GPS取得のリトライ回数
constexpr double GPS_ERROR_THRESHOLD_DEG = 0.0001;  // 要調整：GPS誤差許容範囲(度)
constexpr int    STUCK_RETRY_MAX = 10;              // スタック解消の最大試行回数

const Distination Target = { DEST_LAT, DEST_LON };
Vec2   Here = {0.0, 0.0};           // 現在地(GPSから更新)
Vec2   Nav  = {0.0, 0.0};           // 目的地方向の正規化ベクトル
double lotate_degree = 0.0;         // 機体の目標回転角度
float  DistanceNumberABS = 999.0f;  // ゴールとの絶対距離
bool   ESP32S3 = false;             // 画像認識(ESP32-S3)によるコーン検出フラグ。更新処理はESP32S3SenseReceiver.cpp側に別途実装が必要

void Stuck(int n, float coordlatitude, float coordlongtitude)//nはスタック解消の試行回数事前設定必要
{
    float before_stuckLat = coordlatitude;
    float before_stuckLon = coordlongtitude;

    sendTelemetryText("スタック解消動作を行います");

     gpio_set_level(PIN_RMOTOR_FRONT,1);
     gpio_set_level(PIN_LMOTOR_BACK,1); //反時計回り超信地旋回
    sendTelemetryText("反時計回り超信地旋回を行います");
     vTaskDelay(pdMS_TO_TICKS(2000));

    sendTelemetryText("回転を停止します");
     gpio_set_level(PIN_RMOTOR_FRONT,0); //モータ停止
     gpio_set_level(PIN_LMOTOR_BACK,0); //モータ停止

    sendTelemetryText("時計回り超信地旋回を行います");
     gpio_set_level(PIN_RMOTOR_BACK,1);
     gpio_set_level(PIN_LMOTOR_FRONT,1);
     vTaskDelay(pdMS_TO_TICKS(1000));

    sendTelemetryText("回転を停止します");
     gpio_set_level(PIN_RMOTOR_BACK,0); //モータ停止
     gpio_set_level(PIN_LMOTOR_FRONT,0); //モータ停止

    sendTelemetryText("前進します");
     gpio_set_level(PIN_RMOTOR_FRONT,1);
     gpio_set_level(PIN_RMOTOR_BACK,1);
     vTaskDelay(pdMS_TO_TICKS(4000));

    sendTelemetryText("モータを停止します");
     gpio_set_level(PIN_RMOTOR_FRONT,0); //モータ停止
     gpio_set_level(PIN_RMOTOR_BACK,0); //モータ停止

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
        sendTelemetryText("スタック解消プログラムを終了します");
        return;
    }
    else
    {
        sendTelemetryText("スタック解消完了、スタック解消プログラムを終了します");
        return;
    }
}

void DistanceToGoal()
{
    double dx = Target.x - Here.x;
    double dy = Target.y - Here.y;
    DistanceNumberABS = static_cast<float>(std::sqrt(dx*dx + dy*dy));

    char Kyori[64];
    snprintf(Kyori, sizeof(Kyori), "ゴールとの絶対距離: %.2f", DistanceNumberABS);
    sendTelemetryText(Kyori);
}

void Cul_NAVVec()
{
    taskENTER_CRITICAL(&gps_mux);
    Here.x = g_coordlatitude;
    Here.y = g_coordlongtitude;
    taskEXIT_CRITICAL(&gps_mux);

    //二点間のベクトルを算出
    Nav.x = Target.x - Here.x;
    Nav.y = Target.y - Here.y;

    //正規化
    double norm = std::sqrt(Nav.x*Nav.x + Nav.y*Nav.y);
    if (norm > 1e-9)
    {
        Nav.x /= norm;
        Nav.y /= norm;
    }
}

void LotateCul()
{
    double dot   = Nav.x * g_bodyEast  + Nav.y * g_bodyNorth;
    double cross = Nav.x * g_bodyNorth - Nav.y * g_bodyEast;
    double theta = std::atan2(cross, dot);
    lotate_degree = theta * 180.0 / std::acos(-1.0);

    char msg[64];
    snprintf(msg, sizeof(msg), "機体回転角算出 = %.2f", lotate_degree);
    sendTelemetryText(msg);
}

void loop_Make_Body_lotate()
{
    double lotate_number = getYaw() + lotate_degree;
    if(lotate_number >= -2 && lotate_number <= 2)
    {
        gpio_set_level(PIN_RMOTOR_FRONT,0); //モータ停止
        gpio_set_level(PIN_RMOTOR_BACK,0); //モータ停止
        gpio_set_level(PIN_LMOTOR_FRONT,0); //モータ停止
        gpio_set_level(PIN_LMOTOR_BACK,0); //モータ停止
        sendTelemetryText("機体方向補正回転停止");
        return;
    }
    else if (lotate_number > 2)
    {
        gpio_set_level(PIN_RMOTOR_FRONT,1);
        gpio_set_level(PIN_LMOTOR_BACK,1); //反時計回り超信地旋回
        sendTelemetryText("右回転");
    }
    else
    {
        gpio_set_level(PIN_RMOTOR_BACK,1);
        gpio_set_level(PIN_LMOTOR_FRONT,1); //時計回り超信地旋回
        sendTelemetryText("左回転");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void GPS_STANBY(int retry = GPS_STANBY_MAX_RETRY)
{
    if(retry <= 0)
    {
        sendTelemetryText("位置取得失敗");
        return;
    }

    taskENTER_CRITICAL(&gps_mux);
    float now_lat = g_coordlatitude;
    float now_lon = g_coordlongtitude;
    taskEXIT_CRITICAL(&gps_mux);

    if(std::abs(now_lat - Target.x) < GPS_ERROR_THRESHOLD_DEG &&
       std::abs(now_lon - Target.y) < GPS_ERROR_THRESHOLD_DEG)
    {
        sendTelemetryText("GPS数値取得成功");
        return;
    }
    else
    {
        sendTelemetryText("GPS数値取得失敗");
        vTaskDelay(pdMS_TO_TICKS(1000)); // GPSの更新周期(1秒)分待ってから再試行
        GPS_STANBY(retry - 1);
    }
}

void NavVector()
{
    Cul_NAVVec();
    LotateCul();
    loop_Make_Body_lotate();
}

void loop_main_RUN()
{
    static bool  gpsReady = false;
    static float DistanceABS = 999.0f;

    if (!gpsReady)
    {
        GPS_STANBY();
        gpsReady = true;
    }

    if(ESP32S3 == false && DistanceABS >= 3.0)
    {
        Cul_NAVVec();
        LotateCul();
        loop_Make_Body_lotate();
         taskENTER_CRITICAL(&gps_mux);
         float before_Lat = g_coordlatitude;
         float before_Lon = g_coordlongtitude;
         taskEXIT_CRITICAL(&gps_mux);
        gpio_set_level(PIN_RMOTOR_FRONT,1);
        gpio_set_level(PIN_RMOTOR_BACK,1);
        vTaskDelay(pdMS_TO_TICKS(1000));
         taskENTER_CRITICAL(&gps_mux);
         float after_Lat = g_coordlatitude;
         float after_Lon = g_coordlongtitude;
         taskEXIT_CRITICAL(&gps_mux);
         float distance_RUNNowANDThenLon = std::abs(after_Lat - before_Lat);
         float distance_RUNNowANDThenLat = std::abs(after_Lon - before_Lon);
         DistanceABS = sqrt(distance_RUNNowANDThenLat*distance_RUNNowANDThenLat
            + distance_RUNNowANDThenLon*distance_RUNNowANDThenLon);
        DistanceToGoal();
         if (DistanceABS < 0.5)
        {
          sendTelemetryText("スタックしています。スタック解消動作を行います");
          Stuck(STUCK_RETRY_MAX, g_coordlatitude, g_coordlongtitude);
        }
    }
    if (ESP32S3 == true)
    {
        if (DistanceNumberABS < 1.0)
        {
            sendTelemetryText("ゴールに到着しました.STATEをGOALに移行します");
            currentState = CanSatState::GOAL;
        }
        else
        {
                Cul_NAVVec();
            LotateCul();
            loop_Make_Body_lotate();
                taskENTER_CRITICAL(&gps_mux);
                float before_Lat = g_coordlatitude;
                float before_Lon = g_coordlongtitude;
                taskEXIT_CRITICAL(&gps_mux);
            gpio_set_level(PIN_RMOTOR_FRONT,1);
            gpio_set_level(PIN_RMOTOR_BACK,1);
            vTaskDelay(pdMS_TO_TICKS(1000));
                taskENTER_CRITICAL(&gps_mux);
                float after_Lat = g_coordlatitude;
                float after_Lon = g_coordlongtitude;
                taskEXIT_CRITICAL(&gps_mux);
                float distance_RUNNowANDThenLon = std::abs(after_Lat - before_Lat);
                float distance_RUNNowANDThenLat = std::abs(after_Lon - before_Lon);
                DistanceABS = sqrt(distance_RUNNowANDThenLat*distance_RUNNowANDThenLat
                + distance_RUNNowANDThenLon*distance_RUNNowANDThenLon);
            DistanceToGoal();
                if (DistanceABS < 0.5)
            {
                sendTelemetryText("スタックしています。スタック解消動作を行います");
                Stuck(STUCK_RETRY_MAX, g_coordlatitude, g_coordlongtitude);
            }
        }
    }
    if (ESP32S3 == false && DistanceABS < 3.0)
    {
        sendTelemetryText("コーンを捜索し検知する動作を開始します");
        while (ESP32S3 == false)
        {
            gpio_set_level(PIN_RMOTOR_BACK,1);
            gpio_set_level(PIN_LMOTOR_FRONT,1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (ESP32S3 == true)
        {
            if (DistanceNumberABS < 1.0)
            {
             sendTelemetryText("ゴールに到着しました.STATEをGOALに移行します");
             currentState = CanSatState::GOAL;
            }
            else
            {
                Cul_NAVVec();
                LotateCul();
                loop_Make_Body_lotate();
                    taskENTER_CRITICAL(&gps_mux);
                    float before_Lat = g_coordlatitude;
                    float before_Lon = g_coordlongtitude;
                    taskEXIT_CRITICAL(&gps_mux);
                gpio_set_level(PIN_RMOTOR_FRONT,1);
                gpio_set_level(PIN_RMOTOR_BACK,1);
                vTaskDelay(pdMS_TO_TICKS(1000));
                    taskENTER_CRITICAL(&gps_mux);
                    float after_Lat = g_coordlatitude;
                    float after_Lon = g_coordlongtitude;
                    taskEXIT_CRITICAL(&gps_mux);
                    float distance_RUNNowANDThenLon = std::abs(after_Lat - before_Lat);
                    float distance_RUNNowANDThenLat = std::abs(after_Lon - before_Lon);
                    DistanceABS = sqrt(distance_RUNNowANDThenLat*distance_RUNNowANDThenLat
                    + distance_RUNNowANDThenLon*distance_RUNNowANDThenLon);
                DistanceToGoal();
                    if (DistanceABS < 0.5)
                {
                    sendTelemetryText("スタックしています。スタック解消動作を行います");
                    Stuck(STUCK_RETRY_MAX, g_coordlatitude, g_coordlongtitude);
                }
            }
        }
    }
}
