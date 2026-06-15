#include "CanSat_EachFileConnect.cpp"
#include "PIN_WIRE.cpp"
#include "driver/i2c_master.h"

#include "driver/i2c.h"
#include <cstdint>
#include <cstdio>

// BNO055 I2Cアドレス・レジスタ定義
constexpr uint8_t BNO055_ADDR       = 0x28;
constexpr uint8_t BNO055_OPR_MODE   = 0x3D;
constexpr uint8_t BNO055_EULER_H_LSB = 0x1A;
constexpr uint8_t MODE_NDOF         = 0x0C;

constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
constexpr gpio_num_t SDA_PIN  = GPIO_NUM_21;
constexpr gpio_num_t SCL_PIN  = GPIO_NUM_22;
constexpr uint32_t I2C_FREQ   = 400000;

#include "driver/i2c.h"
#include <cmath>

#define I2C_PORT     I2C_NUM_0
#define BNO055_ADDR  0x28
#define SDA_PIN      21
#define SCL_PIN      22

// BNO055レジスタ
#define REG_OPR_MODE  0x3D
#define REG_QUA_DATA  0x20  // クォータニオン先頭レジスタ

// I2C初期化
void i2cInit() {
    i2c_config_t conf = {};
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = SDA_PIN;
    conf.scl_io_num       = SCL_PIN;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
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

// BNO055初期化（IMUPLUSモード）
void bnoInit() {
    bnoWrite(REG_OPR_MODE, 0x08);  // OPERATION_MODE_IMUPLUS
    vTaskDelay(pdMS_TO_TICKS(100));
}

// Yaw取得（-180°〜+180°）
float getYaw() {
    uint8_t buf[8];
    bnoRead(REG_QUA_DATA, buf, 8);

    // 16bit符号付き整数に変換
    int16_t raw_w = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_x = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_y = (int16_t)((buf[5] << 8) | buf[4]);
    int16_t raw_z = (int16_t)((buf[7] << 8) | buf[6]);

    // スケーリング（BNO055は1/2^14単位）
    const float scale = 1.0f / 16384.0f;
    float qw = raw_w * scale;
    float qx = raw_x * scale;
    float qy = raw_y * scale;
    float qz = raw_z * scale;

    // クォータニオン → Yaw
    return atan2f(2.0f*(qw*qz + qx*qy),
                  1.0f - 2.0f*(qy*qy + qz*qz))
           * 180.0f / M_PI;
}

extern "C" void app_main() {
    i2cInit();
    bnoInit();

    while (1) {
        float yaw = getYaw();
        printf("Yaw: %.2f deg\n", yaw);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}