
Add these 3 files to the kernel:
watchdog.h
watchdog.c
watchdog_port.c

-->Add its declaration to task.h:
int8_t task_get_id(const TCB_t *task);

2. scheduler.c

-->Include the watchdog header:
#include "../Inc/watchdog.h"

-->Start Hardware Watchdog
Inside kernel_start(), after:
port_systick_init(TICK_RATE_HZ);
and before:
port_start_first_task();
add:
kernel_watchdog_start();


3. port.h

Add the hardware watchdog interface:

void port_watchdog_start(void);
void port_watchdog_kick(void);

