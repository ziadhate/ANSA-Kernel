#include <stddef.h>
#include "fsm.h"

/*
 * Production build should route this record to the platform black-box
 * recorder without performing blocking I/O in the 50 Hz FSM task.
 */
typedef struct {
    uint32_t timestamp_us;
    fsm_state_t from;
    fsm_event_type_t event;
    fsm_state_t to;
    fsm_transition_reason_t reason;
} fsm_transition_log_t;

void fsm_logger_record(const fsm_transition_log_t *record)
{
    (void)record;
}
