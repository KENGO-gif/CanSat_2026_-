#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"

// UART設定（PIN_WIRE.hppのGPS用ピン割り当てに準拠）
constexpr uart_port_t GPS_UART_PORT = UART_NUM_1;
constexpr gpio_num_t  GPS_TX_PIN    = TXD_PIN_GPS;
constexpr gpio_num_t  GPS_RX_PIN    = RXD_PIN_GPS;
constexpr int         GPS_BAUD_RATE = 9600;
constexpr int         BUF_SIZE      = 1024;

// グローバル変数（Stuck関数と共有）
volatile float g_coordlatitude  = 0.0f;
volatile float g_coordlongtitude = 0.0f;
portMUX_TYPE gps_mux = portMUX_INITIALIZER_UNLOCKED;

// GPS座標構造体
struct GpsCoordinate {
    double latitude;       // 緯度  (-90.0  〜 +90.0)
    double longitude;      // 経度  (-180.0 〜 +180.0)
    float  altitude;       // 高度  [m]
    float  speed;          // 速度  [km/h]
    float  course;         // 方位角 [degree]
    uint8_t satellites;    // 捕捉衛星数
    bool   is_valid;       // 測位有効フラグ
};

// UART初期化
static esp_err_t uart_init() {
    uart_config_t uart_conf = {};
    uart_conf.baud_rate  = GPS_BAUD_RATE;
    uart_conf.data_bits  = UART_DATA_8_BITS;
    uart_conf.parity     = UART_PARITY_DISABLE;
    uart_conf.stop_bits  = UART_STOP_BITS_1;
    uart_conf.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;

    esp_err_t ret = uart_param_config(GPS_UART_PORT, &uart_conf);
    if (ret != ESP_OK) return ret;

    ret = uart_set_pin(GPS_UART_PORT, GPS_TX_PIN, GPS_RX_PIN,
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) return ret;

    return uart_driver_install(GPS_UART_PORT, BUF_SIZE, 0, 0, nullptr, 0);
}

// NMEA ddmm.mmmm → 十進度数変換
static double nmea_to_decimal(const char *raw, char direction) {
    if (raw == nullptr || raw[0] == '\0') return 0.0;

    double raw_val = atof(raw);
    int    degrees = static_cast<int>(raw_val / 100);
    double minutes = raw_val - (degrees * 100.0);
    double decimal = degrees + (minutes / 60.0);

    if (direction == 'S' || direction == 'W') decimal = -decimal;
    return decimal;
}

// NMEAチェックサム検証
static bool verify_checksum(const char *sentence) {
    if (sentence == nullptr || sentence[0] != '$') return false;

    const char *p    = sentence + 1;
    const char *star = strchr(p, '*');
    if (star == nullptr) return false;

    uint8_t calc = 0;
    while (p != star) calc ^= static_cast<uint8_t>(*p++);

    uint8_t received = static_cast<uint8_t>(strtol(star + 1, nullptr, 16));
    return calc == received;
}

// フィールド切り出し
static const char* get_field(const char *sentence, int index) {
    static char field[32];
    int count = 0;
    const char *p = sentence;

    while (*p && count < index) {
        if (*p++ == ',') count++;
    }

    int i = 0;
    while (*p && *p != ',' && *p != '*' && i < 31) {
        field[i++] = *p++;
    }
    field[i] = '\0';
    return field;
}

// $GPRMC パース
static bool parse_gprmc(const char *sentence, GpsCoordinate &coord) {
    if (strncmp(sentence, "$GPRMC", 6) != 0) return false;
    if (!verify_checksum(sentence)) return false;

    const char status = get_field(sentence, 2)[0];
    coord.is_valid    = (status == 'A');

    coord.latitude  = nmea_to_decimal(get_field(sentence, 3),
                                      get_field(sentence, 4)[0]);
    coord.longitude = nmea_to_decimal(get_field(sentence, 5),
                                      get_field(sentence, 6)[0]);

    coord.speed  = static_cast<float>(atof(get_field(sentence, 7))) * 1.852f;
    coord.course = static_cast<float>(atof(get_field(sentence, 8)));

    return true;
}

// $GPGGA パース
static bool parse_gpgga(const char *sentence, GpsCoordinate &coord) {
    if (strncmp(sentence, "$GPGGA", 6) != 0) return false;
    if (!verify_checksum(sentence)) return false;

    coord.satellites = static_cast<uint8_t>(atoi(get_field(sentence, 7)));
    coord.altitude   = static_cast<float>(atof(get_field(sentence, 9)));

    return true;
}

// UART1行読み取り
static bool read_nmea_line(char *buf, int max_len) {
    int  idx = 0;
    char c   = 0;

    while (idx < max_len - 1) {
        int len = uart_read_bytes(GPS_UART_PORT,
                                  reinterpret_cast<uint8_t*>(&c),
                                  1, pdMS_TO_TICKS(1000));
        if (len <= 0) return false;
        if (c == '\n') break;
        if (c != '\r') buf[idx++] = c;
    }
    buf[idx] = '\0';
    return idx > 0;
}

// GPS座標取得
static esp_err_t getGpsCoordinate(GpsCoordinate &coord) {
    char line[BUF_SIZE];
    bool got_rmc = false;
    bool got_gga = false;

    for (int attempt = 0; attempt < 20; ++attempt) {
        if (!read_nmea_line(line, sizeof(line))) continue;

        if (!got_rmc && parse_gprmc(line, coord)) got_rmc = true;
        if (!got_gga && parse_gpgga(line, coord)) got_gga = true;

        if (got_rmc && got_gga) return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

// GPSタスク（別タスクとして常時動作）
void gpsTask(void *pvParameters) {
    GpsCoordinate coord = {};

    while (true) {
        if (getGpsCoordinate(coord) == ESP_OK && coord.is_valid) {

            // グローバル変数をmutexで保護して更新
            taskENTER_CRITICAL(&gps_mux);
            g_coordlatitude   = static_cast<float>(coord.latitude);
            g_coordlongtitude = static_cast<float>(coord.longitude);
            taskEXIT_CRITICAL(&gps_mux);

            // 地上局へGPSテレメトリを送信（ESP-NOW経由、telemetry.cppに委譲）
            sendGpsTelemetry(coord.satellites, coord.latitude,
                             coord.longitude, coord.altitude);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// GPSタスク起動（main側のapp_mainから呼び出す）
void startGpsTask() {
    ESP_ERROR_CHECK(uart_init());
    xTaskCreate(gpsTask, "gpsTask", 4096, nullptr, 5, nullptr);
}

