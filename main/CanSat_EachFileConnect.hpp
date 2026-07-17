//このプログラムの目的はあらゆるファイルで共通の関数を留め置きし、共有すること

#pragma once

#include <cstdint>
#include "freertos/FreeRTOS.h"

enum class CanSatState
{
    STANDBY,
    FRYING,
    START,
    RUN,
    GOAL
};

extern CanSatState currentState;
// CanSat_EachFileConnect.hpp
struct AccelData { float x, y, z; };
extern AccelData g_accel;
void updateAccel();

void sendTelemetryText(const char* text);
void sendGpsTelemetry(uint8_t satellites, double latitude, double longitude, float altitude);
void setTelemetryChannel(uint8_t channel);

// ---- GPS用コード.cpp ----
extern volatile float g_coordlatitude;
extern volatile float g_coordlongtitude;
extern portMUX_TYPE gps_mux;
void startGpsTask();

// ---- CanSatState.cpp ----
extern CanSatState currentState;
void CanSat_state_logic();

// ---- ProGram_STANDBY.cpp ----
void loop_STANBY();

// ---- ProGram_FRYING.cpp ----
void setup_FRYING();
void loop_FRYING();

// ---- ProGram_START.cpp ----
void setup_START();
void loop_START();

// ---- ProGram_RUN.cpp ----
void Stuck(int n, float coordlatitude, float coordlongtitude);
void Cul_NAVVec(const Vec2 Target, const Vec2 Here);
void NavVector();

// ---- ProGram_GOAL.cpp ----
void loop_GOAL();

