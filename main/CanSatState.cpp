#include "CanSatState.hpp"
#include "CanSat_EachFileConnect.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern void loop_main_RUN(); // ProGram_RUN.cpp（未だCanSat_EachFileConnect.hppに未登録のためここで宣言）

CanSatState currentState = CanSatState::STANDBY;

void CanSat_state_logic()
{
    while (1)
    {
        switch (currentState)
        {
            case CanSatState::STANDBY:
            loop_STANBY();
            break;

            case CanSatState::FRYING:
            setup_FRYING();
            loop_FRYING();
            break;

            case CanSatState::START:
            setup_START();
            loop_START();
            break;

            case CanSatState::RUN:
            loop_main_RUN();
            break;

            case CanSatState::GOAL:
            loop_GOAL();
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}