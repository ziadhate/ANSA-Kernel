#include <stdint.h>

#include "kernel.h"
#include "task.h"
#include "queue.h"


/* =========================================================
 * Queue
 * ========================================================= */

static kernel_queue_t queue;


/* =========================================================
 * Data
 * ========================================================= */

static uint32_t data = 0;

static volatile uint32_t received_data = 0;


/* =========================================================
 * Producer
 * ========================================================= */

static void Producer_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        data++;

        kernel_queue_send(
            &queue,
            &data
        );

        task_delay(500);
    }
}


/* =========================================================
 * Consumer
 * ========================================================= */

static void Consumer_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        if (kernel_queue_receive(
                &queue,
                &received_data))
        {
            /*
             * Put breakpoint here.
             *
             * received_data:
             * 1
             * 2
             * 3
             * 4
             * ...
             */
        }

        task_delay(100);
    }
}


/* =========================================================
 * Main
 * ========================================================= */

int main(void)
{
    kernel_init();

    /*
     * Queue:
     *
     * Element size = uint32_t
     * Number of elements = 8
     */

    kernel_queue_init(
        &queue,
        sizeof(uint32_t),
        8
    );


    kernel_task_create(
        Producer_Task,
        NULL,
        1,
        256,
        0
    );


    kernel_task_create(
        Consumer_Task,
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
