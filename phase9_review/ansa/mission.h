#ifndef MISSION_H
#define MISSION_H

#include <stdbool.h>
void mission_init(void);
void mission_start(void);
void mission_execute(void);
void mission_stop(void);
void mission_completed(void);

#endif // MISSION_H