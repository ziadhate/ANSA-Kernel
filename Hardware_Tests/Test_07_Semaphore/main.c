#include <stdint.h>

#include "kernel.h"
#include "task.h"
#include "semaphore.h"


/* =========================================================
 * Semaphore
 * ========================================================= */

static kernel_sem_t semaphore;


/* =========================================================
 * Counters
 * ========================================================= */

static volatile uint32_t task_a_counter = 0;

static volatile uint32_t task_b_counter = 0;


/* =========================================================
 * Task A
 * ========================================================= */

static void TaskA(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Take semaphore
         */
        kernel_sem_take(&semaphore);


        /*
         * Critical Section
         */
        task_a_counter++;


        /*
         * Release semaphore
         */
        kernel_sem_give(&semaphore);


        task_delay(500);
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
        kernel_sem_take(&semaphore);

        /*
         * Critical Section
         */
        task_b_counter++;

        kernel_sem_give(&semaphore);

        task_delay(700);
    }
}


/* =========================================================
 * Main
 * ========================================================= */

int main(void)
{
    kernel_init();


    /*
     * Binary semaphore
     */
    kernel_sem_init(
        &semaphore,
        1
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
