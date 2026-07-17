#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ESP_NOW_SENDER";
static uint8_t broadcast_address[] = {0x68, 0x25, 0xDD, 0xBB, 0x51, 0x90};
static bool is_radio_active = false;
static uint8_t current_channel = 1;

typedef struct {
    unsigned long time_stamp;
    char status[200];
} struct_message;

// ★コード2のstruct_message相当（GPS情報一式をESP-NOWで送信）
typedef struct {
    uint8_t channel;
    uint8_t satellites;
    double  latitude;
    double  longitude;
    float   altitude;
} gps_message;

static void apply_wifi_channel(uint8_t ch)
{
    if (ch < 1 || ch > 13) return;

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    current_channel = ch;
    ESP_LOGI(TAG, "WiFi channel switched to: %d", current_channel);
}

// ★コード2のhandleSerialInput相当（"CH<n>"でESP-NOWチャンネルを切替）
static void telemetryConsoleTask(void *pvParameters)
{
    char line[32];
    while (true) {
        if (fgets(line, sizeof(line), stdin) != nullptr) {
            if (strncmp(line, "CH", 2) == 0) {
                int new_ch = atoi(line + 2);
                if (new_ch >= 1 && new_ch <= 13) {
                    apply_wifi_channel(static_cast<uint8_t>(new_ch));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void start_esp_now(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    apply_wifi_channel(current_channel);
    ESP_ERROR_CHECK(esp_now_init());

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcast_address, 6);
    peer.channel = 0;   // 0 = 現在のWiFiチャンネルに追従
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    xTaskCreate(telemetryConsoleTask, "telemetryConsole", 2048, nullptr, 4, nullptr);

    is_radio_active = true;
}

void setTelemetryChannel(uint8_t channel)
{
    if (channel < 1 || channel > 13) return;
    if (is_radio_active) {
        apply_wifi_channel(channel);
    } else {
        current_channel = channel;
    }
}

void sendTelemetryText(const char *text)
{
    if (!is_radio_active) start_esp_now();

    struct_message tx = {};
    tx.time_stamp = (unsigned long)(esp_timer_get_time() / 1000ULL);
    strncpy(tx.status, text, sizeof(tx.status) - 1);

    ESP_LOGI(TAG, "Sending: %s", text);

    esp_err_t result = esp_now_send(broadcast_address, (uint8_t *)&tx, sizeof(tx));
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Send FAIL: %s", esp_err_to_name(result));
    } else {
        ESP_LOGI(TAG, "Send OK");
    }
}

// ★コード2のloop()内送信部相当（GPS用コード.cppのgpsTaskから毎周期呼ばれる）
void sendGpsTelemetry(uint8_t satellites, double latitude, double longitude, float altitude)
{
    if (!is_radio_active) start_esp_now();

    gps_message tx = {};
    tx.channel    = current_channel;
    tx.satellites = satellites;
    tx.latitude   = latitude;
    tx.longitude  = longitude;
    tx.altitude   = altitude;

    esp_err_t result = esp_now_send(broadcast_address, (uint8_t *)&tx, sizeof(tx));
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "GPS Send FAIL: %s", esp_err_to_name(result));
    } else {
        ESP_LOGI(TAG, "GPS Send OK: sat=%u lat=%.6f lon=%.6f alt=%.1f",
                satellites, latitude, longitude, altitude);
    }
}