// 本体プロジェクト(main/PIN_WIRE.hpp)から、最終誘導(画像認識)テストに必要なピンのみ抜粋
// 本番機と同じ配線・ピン番号でテストできるよう、定義値は本体と揃えてある

#pragma once
#include "driver/gpio.h"

#define PIN_RMOTOR_FRONT     GPIO_NUM_2   // 右モーター順転
#define PIN_RMOTOR_BACK      GPIO_NUM_4   // 右モーター反転
#define PIN_LMOTOR_FRONT     GPIO_NUM_32  // 左モーター順転
#define PIN_LMOTOR_BACK      GPIO_NUM_33  // 左モーター反転

#define TXD_PIN_S3sense      GPIO_NUM_27  // UART_TX_ESP32S3sense（本体同様、現状未使用）
#define RXD_PIN_S3sense      GPIO_NUM_14  // UART_RX_ESP32S3sense（コーン認識結果 HIGH=認識中）
