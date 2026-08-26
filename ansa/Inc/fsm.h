#ifndef FSM_M
#define FSM_M

#include <stdbool.h>

//States
typedef enum{
    STATE_BOOT = 0,
    STATE_IDLE,
    STATE_ARMING,
    STATE_ARMED,
    STATE_TAKEOFF,
    STATE_HOVER,
    STATE_MISSION,
    STATE_RTL,
    STATE_LAND,
    STATE_EMERGENCY,
    STATE_FAILSAFE
}State_t;

//Events
typedef enum{
    EVT_NONE = 0,
    EVT_INIT_OK,
    EVT_ARM_CMD,
    EVT_DISARM_CMD,
    EVT_TAKEOFF_CMD,
    EVT_MISSION_CMD,
    EVT_MISSION_COMPLETE,
    EVT_HOME_REACHED,
    EVT_SIGNAL_LOST,
    EVT_SENSOR_FAIL,
    EVT_BATTERY_CRITICAL
}Event_t;

//Sensor Data
typedef struct{
    float altitude_m;
    float heading_deg;
    bool imu_ok;
    bool baro_ok;
    bool mag_ok;
    float battery_percent;
}SensorData_t;

//FSM API
void fsm_init(void);
void fsm_update_sensor_data(const SensorData_t *data);
void fsm_tick(Event_t evt);
State_t fsm_get_state(void);

#endif
