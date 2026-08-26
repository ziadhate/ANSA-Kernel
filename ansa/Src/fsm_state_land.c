#include "fsm_internal.h"

void fsm_state_land_enter(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_LAND;
}

void fsm_state_land_periodic(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_LAND;
}

void fsm_state_land_exit(void)
{
}
