#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"

static const char *TAG = "ESP_NOW_SENDER";
static uint8_t broadcast_address[] = {0x68, 0x25, 0xDD, 0xBB, 0x51, 0x90};
static bool is_radio_active = false;

typedef struct {
    unsigned long time_stamp;
    char status[16];
} struct_message;

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
    ESP_ERROR_CHECK(esp_now_init());

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcast_address, 6);
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    is_radio_active = true;
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