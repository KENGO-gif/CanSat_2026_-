#include "CanSatState.hpp"
#include "CanSat_EachFileConnect.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

            case CanSatState::LANDING:
            setup_LANDING();
            loop_LANDING();
            break;

            case CanSatState::START:
            setup_START();
            loop_START();
            break;

            case CanSatState::RUN:
            loop_RUN();
            break;

            case CanSatState::GOAL:
            loop_GOAL();
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}