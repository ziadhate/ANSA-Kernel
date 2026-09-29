#include "fsm_internal.h"

void fsm_state_armed_enter(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_ATTITUDE;
}

void fsm_state_armed_periodic(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_ATTITUDE;
}

void fsm_state_armed_exit(void)
{
}
