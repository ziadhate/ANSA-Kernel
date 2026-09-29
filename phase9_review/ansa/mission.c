#include <stdio.h>
#include "mission.h"

static int waypoint;
static bool complete;

void mission_init(void)
{
    waypoint = 0;
    complete = false;
}

void mission_start(void)
{
    waypoint = 1;
    complete = false;
    printf("[MISSION] Started\n");
}

