#include "tasks.h"
// #include "scheduler.h" // update the task priority
#include "fsm/event_queue.h"
#include "fsm/flight_fsm.h"
#include "logger.h"
#include "shared_state.h"

#include "sensors_api/barometer/barometer.h"
#include "sensors_api/imu/imu.h"
#include "sensors_api/magnetometer/magnetometer.h"

#include <stdbool.h>

void task_sensor_read_and_update(void) {
  /* Update sensor in order: magnetometer, IMU, barometer */
  MAGNETOMETER_update();
  IMU_update();
  BAROMETER_update();

  /* Publish latest filtered values to shared system state */
  g_sensor_state.pitch = IMU_getPitch();
  g_sensor_state.roll = IMU_getRoll();
  g_sensor_state.yaw = IMU_getYaw();
  g_sensor_state.altitude = BAROMETER_getAltitude();

  LOG_DEBUG("SENSOR state | pitch=%.2f roll=%.2f yaw=%.2f alt=%.2f",
            g_sensor_state.pitch, g_sensor_state.roll, g_sensor_state.yaw,
            g_sensor_state.altitude);
}

void task_fsm_command_simulation(void) {
  switch (g_fsm.mode) {
  case DISARMED:
    LOG_DEBUG("SIM pushing EVENT_ARM");
    FSM_pushEvent(EVENT_ARM);
    break;

  case ARMED:
    LOG_DEBUG("SIM pushing EVENT_TAKEOFF");
    FSM_pushEvent(EVENT_TAKEOFF);
    break;

  case TAKEOFF:
    LOG_DEBUG("SIM pushing EVENT_TAKEOFF_COMPLETE");
    FSM_pushEvent(EVENT_TAKEOFF_COMPLETE);
    break;

  case HOVER:
    LOG_DEBUG("SIM pushing EVENT_LAND");
    FSM_pushEvent(EVENT_LAND);
    break;

  case LAND:
    LOG_DEBUG("SIM pushing EVENT_LANDING_COMPLETE");
    FSM_pushEvent(EVENT_LANDING_COMPLETE);
    break;

  case FAILSAFE:
    LOG_DEBUG("SIM pushing EVENT_LAND");
    FSM_pushEvent(EVENT_LAND);
    break;

  default:
    break;
  }
}

void task_fsm_update(void) {
  FSM_update();

  LOG_DEBUG("FSM state | mode=%s target_pitch=%.2f target_roll=%.2f "
            "target_yaw=%.2f target_alt=%.2f",
            FSM_getStateName(g_fsm.mode), g_fsm.target_pitch, g_fsm.target_roll,
            g_fsm.target_yaw, g_fsm.target_alt);
}

void task_pid_control(void) { LOG_DEBUG("PID | pid task run"); }

void task_motor_control(void) { LOG_DEBUG("MOTOR | motor task run"); }
