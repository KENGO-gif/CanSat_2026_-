//MOTTII_CanSat_ESP32におけるPIN指定を支持する
//このプログラムは全ての処理を従属し、センサ、出力の経路を指定する

#pragma once
#include "driver/gpio.h"

// --- GPIO PIN ASSIGNMENT ---
// --- ESP32C6 ---
// --- CanSat Nosiro ---

#define PIN_RMOTOR_FRONT     GPIO_NUM_2  // 右モーター順転
#define PIN_RMOTOR_BACK      GPIO_NUM_4  // 右モーター反転
#define PIN_LMOTOR_FRONT     GPIO_NUM_32  // 左モーター順転
#define PIN_LMOTOR_BACK      GPIO_NUM_33  // 左モーター反転

#define PIN_NICROM           GPIO_NUM_16  // ニクロム線加熱用

#define I2C_MASTER_SCL_IO    GPIO_NUM_22  // 9軸SCL,電流検知
#define I2C_MASTER_SDA_IO    GPIO_NUM_21  // 9軸SDA,電流検知

#define TXD_PIN_GPS          GPIO_NUM_27   // URAT_TX_GPS用
#define RXD_PIN_GPS          GPIO_NUM_14   // URAT_RX_GPS用
#define TXD_PIN_S3sense      GPIO_NUM_25  // URAT_TX_ESP32S3sense
#define RXD_PIN_S3sense      GPIO_NUM_26  // URAT_RX_ESP32S3sense

#define FALLOUT_PIN_BLUP     GPIO_NUM_13   //ピン抜けJAMP BullUp
#define FALLOUT_PIN_GND      GPIO_NUM_1   //ピン抜けJAMP GND

 

// ---------------------------
