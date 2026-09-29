#include <string.h>
#include "fsm_internal.h"

void fsm_state_boot_enter(uint32_t now_us)
{
    (void)now_us;
    memset(&g_command, 0, sizeof(g_command));
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_DISARMED;
}

void fsm_state_boot_periodic(uint32_t now_us)
{
    (void)now_us;
}

void fsm_state_boot_exit(void)
{
}
