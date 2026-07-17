//このプログラムは機体のゴール検知とそのテレメトり処理の指定を担当する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void loop_GOAL()
{
  while(true)
  {
    sendTelemetryText("CanSat GOAL!!!");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}