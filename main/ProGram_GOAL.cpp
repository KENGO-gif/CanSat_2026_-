//このプログラムは機体のゴール検知とそのテレメトり処理の指定を担当する

#include "CanSat_EachFileConnect.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void loop_GOAL()
{
    sendTelemetryText("CanSat GOAL!!!");
    vTaskDelay(pdMS_TO_TICKS(1000));

}

// 関数を追加
float getStableDistance(int samples) {
  float total = 0;
  int valid = 0;
  for (int i = 0; i < samples; i++) {
    float d = measureDistance();
    if (d > 0) {
      total += d;
      valid++;
    }
    delay(10);
  }
  if (valid == 0) return -1.0;
  return total / valid;
}