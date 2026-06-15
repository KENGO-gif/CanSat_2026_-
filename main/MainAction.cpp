//このプログラムはメイン処理を担当しており、機体の遷移状態を決定することで
//アクションを指定する

#include <stdio.h>
#include "CanSat_EachFileConnect.hpp"
#include "CanSatState.hpp"
#include "PIN_WIRE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void)
{
    CanSat_state_logic();
}


