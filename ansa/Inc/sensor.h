#ifndef SENSOR_H
#define SENSOR_H

#include "fsm.h"

void sensor_init(void);

void sensor_task(void);

SensorData_t sensor_get_data(void);

#endif