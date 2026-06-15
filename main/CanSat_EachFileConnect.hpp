//このプログラムの目的はあらゆるファイルで共通の関数を留め置きし、共有すること

#pragma once


enum class CanSatState
{
    STANDBY,
    LANDING,
    START,
    GOAL
};

extern CanSatState currentState;

void loop_START();
void setup_START();
void loop_STANBY();
void loop_GOAL();
void CanSat_state_logic();
void sendTelemetryText(const char* text);
void setup_LANDING();
void loop_LANDING();
void STANBY_main();
void app_MAINACTION();

