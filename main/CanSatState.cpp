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

            case CanSatState::FRYING:
            setup_FRYING();
            loop_FRYING();
            break;

            case CanSatState::START:
            setup_START();
            loop_START();
            break;

            case CanSatState::RUN:
            Stuck(int n, float coordlatitude, float coordlongtitude);
            loop_RUN();
            break;

            case CanSatState::GOAL:
            loop_GOAL();
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}