#include <stdint.h>

#include "kernel.h"
#include "task.h"
#include "mutex.h"


/* =========================================================
 * Mutex
 * ========================================================= */

static kernel_mutex_t mutex;


/* =========================================================
 * Shared Resource
 * ========================================================= */

static volatile uint32_t shared_resource = 0;


/* =========================================================
 * Task A
 * ========================================================= */

static void TaskA(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Lock shared resource
         */
        kernel_mutex_lock(&mutex);


        /*
         * Critical Section
         */
        shared_resource++;


        /*
         * Unlock
         */
        kernel_mutex_unlock(&mutex);


        task_delay(300);
    }
}


/* =========================================================
 * Task B
 * ========================================================= */

static void TaskB(void *arg)
{
    (void)arg;

    while (1)
    {
        kernel_mutex_lock(&mutex);

        /*
         * Critical Section
         */
        shared_resource++;

        kernel_mutex_unlock(&mutex);

        task_delay(500);
    }
}


/* =========================================================
 * Main
 * ========================================================= */

int main(void)
{
    kernel_init();


    kernel_mutex_init(
        &mutex
    );


    kernel_task_create(
        TaskA,
        NULL,
        1,
        256,
        0
    );


    kernel_task_create(
        TaskB,
        NULL,
        1,
        256,
        0
    );


    kernel_start();

    while (1)
    {
    }

    return 0;
}
