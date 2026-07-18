#include "CanSat_EachFileConnect.hpp"   // .cpp → .hppに修正
#include "PIN_WIRE.hpp"                 // .cpp → .hppに修正(右側のファイル)
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "IMU";

// BNO055のインスタンス作成 (第1引数は識別用ID、第2引数はI2Cアドレス: デフォルトは0x28)
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
static bool s_bnoReady = false;

// ---- 加速度データ ----
// AccelData構造体・g_accelはCanSat_EachFileConnect.hppで宣言済みのためここでは実体定義のみ行う
AccelData g_accel;
portMUX_TYPE accel_mux = portMUX_INITIALIZER_UNLOCKED;    // 排他制御用

// 実際のapp_main()はmain/MainAction.cppに一本化されているため、
// このファイルではArduino/Adafruitランタイムの初期化を提供する。
// (単体テストプロジェクトimu_testで動作確認済みの初期化手順をそのまま移植)
void initImu() {
    initArduino();  // arduino-esp32コンポーネントの初期化(Wire等のArduino APIを使うために必要)

    Wire.begin((int)I2C_MASTER_SDA_IO, (int)I2C_MASTER_SCL_IO);   // PIN_WIRE.hpp由来

    if (!bno.begin()) {
        ESP_LOGE(TAG, "BNO055 not detected. Check wiring/I2C address.");
        s_bnoReady = false;
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    bno.setExtCrystalUse(true);

    s_bnoReady = true;
    ESP_LOGI(TAG, "BNO055 initialized successfully");
}

// 加速度取得・更新(単位: m/s^2、重力込み)
void updateAccel() {
    if (!s_bnoReady) {
        ESP_LOGW(TAG, "updateAccel() called before BNO055 was ready");
        return;
    }

    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

    taskENTER_CRITICAL(&accel_mux);
    g_accel.x = accel.x();
    g_accel.y = accel.y();
    g_accel.z = accel.z();
    taskEXIT_CRITICAL(&accel_mux);
}

// Yaw(方位)取得
// 注意: 以前の自作クォータニオン実装は-180°〜+180°で返していたが、
// Adafruitライブラリのオイラー角(heading)は0°〜360°で返る点が異なる。
// 呼び出し側(ProGram_START.cpp等)で範囲の違いが問題になる場合は要調整。
float getYaw() {
    if (!s_bnoReady) return 0.0f;
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    return euler.x();
}
