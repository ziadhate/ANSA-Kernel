#ifndef SPACEBORN_FSM_FAULTS_H
#define SPACEBORN_FSM_FAULTS_H

#include <stdint.h>

typedef enum {
    FSM_FAULT_NONE = 0,
    FSM_FAULT_IMU_DIVERGENCE,
    FSM_FAULT_GPS_LOST,
    FSM_FAULT_BAROMETER,
    FSM_FAULT_BATTERY_CRITICAL,
    FSM_FAULT_LINK_LOST,
    FSM_FAULT_MOTOR_SATURATION,
    FSM_FAULT_WATCHDOG,
    FSM_FAULT_GEOFENCE,
    FSM_FAULT_SENSOR_STALE,
    FSM_FAULT_QUEUE_OVERFLOW,
    FSM_FAULT_COUNT
} fsm_fault_code_t;

typedef enum {
    FSM_FAULT_MINOR = 0,
    FSM_FAULT_MAJOR,
    FSM_FAULT_CRITICAL
} fsm_fault_severity_t;

typedef struct {
    uint32_t version;
    fsm_fault_code_t code;
    fsm_fault_severity_t severity;
    uint32_t timestamp_us;
    uint32_t flags;
} fsm_fault_t;

#endif
