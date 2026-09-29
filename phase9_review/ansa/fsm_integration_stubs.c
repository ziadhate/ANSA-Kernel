#include "fsm.h"

#if defined(__GNUC__)
#define FSM_WEAK __attribute__((weak))
#else
#define FSM_WEAK
#endif

FSM_WEAK void fsm_control_publish(
    const fsm_control_command_t *command)
{
    (void)command;
}

FSM_WEAK void fsm_telemetry_publish(
    const fsm_telemetry_t *telemetry)
{
    (void)telemetry;
}

FSM_WEAK void fsm_mission_runtime_on_start(void) {}
FSM_WEAK void fsm_mission_runtime_on_stop(void) {}
