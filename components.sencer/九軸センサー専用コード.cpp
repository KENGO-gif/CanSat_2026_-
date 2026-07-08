#include "CanSat_EachFileConnect.hpp"   // .cpp → .hppに修正
#include "PIN_WIRE.hpp"                 // .cpp → .hppに修正(右側のファイル)
#include "driver/i2c_master.h"
#include "driver/i2c.h"
#include <cstdint>
#include <cstdio>
#include <cmath>

// ---- BNO055 固有の定義(センサー専用ファイルで管理) ----
constexpr uint8_t BNO055_ADDR        = 0x28;
constexpr uint8_t REG_OPR_MODE       = 0x3D;
constexpr uint8_t REG_EULER_H_LSB    = 0x1A;
constexpr uint8_t MODE_NDOF          = 0x0C;
constexpr uint8_t REG_QUA_DATA       = 0x20;  // クォータニオン先頭レジスタ
constexpr uint8_t REG_ACC_DATA       = 0x08;  // 加速度先頭レジスタ

constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
constexpr uint32_t   I2C_FREQ = 400000;
// SDA/SCLは PIN_WIRE.hpp の I2C_MASTER_SDA_IO / I2C_MASTER_SCL_IO を使う(重複定義を廃止)

// I2C初期化
void i2cInit() {
    i2c_config_t conf = {};
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = I2C_MASTER_SDA_IO;   // PIN_WIRE.hpp由来
    conf.scl_io_num       = I2C_MASTER_SCL_IO;   // PIN_WIRE.hpp由来
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ;
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}

// レジスタ書き込み
void bnoWrite(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_master_write_to_device(I2C_PORT, BNO055_ADDR, buf, 2,
                               pdMS_TO_TICKS(10));
}

// レジスタ読み込み
void bnoRead(uint8_t reg, uint8_t *dst, size_t len) {
    i2c_master_write_read_device(I2C_PORT, BNO055_ADDR,
                                 &reg, 1, dst, len,
                                 pdMS_TO_TICKS(10));
}

// BNO055初期化(IMUPLUSモード)
void bnoInit() {
    bnoWrite(REG_OPR_MODE, 0x08);  // OPERATION_MODE_IMUPLUS
    vTaskDelay(pdMS_TO_TICKS(100));
}

// Yaw取得(-180°〜+180°)
float getYaw() {
    uint8_t buf[8];
    bnoRead(REG_QUA_DATA, buf, 8);

    int16_t raw_w = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_x = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_y = (int16_t)((buf[5] << 8) | buf[4]);
    int16_t raw_z = (int16_t)((buf[7] << 8) | buf[6]);

    const float scale = 1.0f / 16384.0f;
    float qw = raw_w * scale;
    float qx = raw_x * scale;
    float qy = raw_y * scale;
    float qz = raw_z * scale;

    return atan2f(2.0f*(qw*qz + qx*qy),
                  1.0f - 2.0f*(qy*qy + qz*qz))
           * 180.0f / M_PI;
}

// ---- 加速度データ ----
struct AccelData { float x, y, z; };
AccelData g_accel;                                       // 他ファイルからも使うならextern化してヘッダーに宣言
portMUX_TYPE accel_mux = portMUX_INITIALIZER_UNLOCKED;    // 排他制御用

// 加速度取得・更新(単位: m/s^2)
void updateAccel() {
    uint8_t buf[6];
    bnoRead(REG_ACC_DATA, buf, 6);   // 前回追加した REG_ACC_DATA = 0x08 を使用

    int16_t raw_x = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_y = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_z = (int16_t)((buf[5] << 8) | buf[4]);

    const float scale = 1.0f / 100.0f;  // BNO055デフォルト単位: 1 LSB = 1/100 m/s^2

    taskENTER_CRITICAL(&accel_mux);
    g_accel.x = raw_x * scale;
    g_accel.y = raw_y * scale;
    g_accel.z = raw_z * scale;
    taskEXIT_CRITICAL(&accel_mux);
}

extern "C" void app_main() {
    i2cInit();
    bnoInit();

    while (1) {
        float yaw = getYaw();
        updateAccel();

        printf("Yaw: %.2f deg | Accel: %.2f, %.2f, %.2f m/s^2\n",
               yaw, g_accel.x, g_accel.y, g_accel.z);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}