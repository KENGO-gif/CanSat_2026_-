//このプログラムはメイン処理を担当しており、機体の遷移状態を決定することで
//アクションを指定する

#include <stdio.h>
#include "CanSat_EachFileConnect.hpp"
#include "CanSatState.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern void initImu(); // 九軸センサー専用コード.cpp（現状CanSat_EachFileConnect.hpp未登録のためここで宣言）

extern "C" void app_main(void)
{
    initImu();
    startGpsTask();
    CanSat_state_logic();
}


