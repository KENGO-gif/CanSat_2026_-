#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

void Gazouhantei()
{
    while(true)
    {
        if (RXD_PIN_S3sense == 1)
        {
            ESP32S3 = true;
        }
        else if (RXD_PIN_S3sense == 0 )
        {
            ESP32S3 = false;
        }
    }
}