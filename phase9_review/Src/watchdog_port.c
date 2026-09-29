#include "../Port/Cortex_M4/Inc/port.h"
#include "watchdog_port.h"
#include <stdint.h>

//needs to be called inthe main function
void port_watchdog_start(void)
{
    /* Start IWDG */
    IWDG_KR = IWDG_KEY_ENABLE;

    /* Unlock PR and RLR */
    IWDG_KR = IWDG_KEY_UNLOCK;

    /* Configure prescaler */
    IWDG_PR = IWDG_PRESCALER;

    /* Configure reload */
    IWDG_RLR = IWDG_RELOAD_VALUE;

    //Initial reload.
    IWDG_KR = IWDG_KEY_RELOAD;
}


void port_watchdog_kick(void){
    IWDG_KR = IWDG_KEY_RELOAD;
}