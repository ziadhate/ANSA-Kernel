#include <stdint.h>

#include "kernel.h"
#include "task.h"
#include "queue.h"
#include "semaphore.h"
#include "mutex.h"


/* =========================================================
 * STM32F401 GPIO
 * ========================================================= */

#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL

#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

#define GPIOAEN         (1U << 0)

#define PA5             (1U << 5)
#define PA6             (1U << 6)


/* =========================================================
 * IPC Objects
 * ========================================================= */

static kernel_queue_t queue;

static kernel_sem_t semaphore;

static kernel_mutex_t mutex;


/* =========================================================
 * Shared Data
 * ========================================================= */

static uint32_t message = 0;

static volatile uint32_t received_message = 0;

static volatile uint32_t shared_resource = 0;

static volatile uint32_t semaphore_counter = 0;


/* =========================================================
 * GPIO Initialization
 * ========================================================= */

static void GPIO_Init(void)
{
    RCC_AHB1ENR |= GPIOAEN;


    /* PA5 Output */

    GPIOA_MODER &= ~(3U << (5 * 2));

    GPIOA_MODER |= (1U << (5 * 2));


    /* PA6 Output */

    GPIOA_MODER &= ~(3U << (6 * 2));

    GPIOA_MODER |= (1U << (6 * 2));
}


/* =========================================================
 * Task 1
 *
 * Producer
 * ========================================================= */

static void Producer_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        message++;


        /*
         * Send message to Queue
         */

        kernel_queue_send(
            &queue,
            &message
        );


        /*
         * LED indication
         */

        GPIOA_ODR ^= PA5;


        /*
         * Delay
         */

        task_delay(500);
    }
}


/* =========================================================
 * Task 2
 *
 * Consumer
 * ========================================================= */

static void Consumer_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        if (kernel_queue_receive(
                &queue,
                &received_message))
        {
            /*
             * Message successfully received
             *
             * received_message =
             * 1, 2, 3, 4, ...
             */

            GPIOA_ODR ^= PA6;
        }


        task_delay(100);
    }
}


/* =========================================================
 * Task 3
 *
 * Semaphore
 * ========================================================= */

static void Semaphore_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Take semaphore
         */

        kernel_sem_take(
            &semaphore
        );


        /*
         * Critical Section
         */

        semaphore_counter++;


        /*
         * Release semaphore
         */

        kernel_sem_give(
            &semaphore
        );


        task_delay(300);
    }
}


/* =========================================================
 * Task 4
 *
 * Mutex Task A
 * ========================================================= */

static void Mutex_Task_A(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Lock Mutex
         */

        kernel_mutex_lock(
            &mutex
        );


        /*
         * Shared Resource
         */

        shared_resource++;


        /*
         * Unlock
         */

        kernel_mutex_unlock(
            &mutex
        );


        task_delay(400);
    }
}


/* =========================================================
 * Task 5
 *
 * Mutex Task B
 * ========================================================= */

static void Mutex_Task_B(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Lock Mutex
         */

        kernel_mutex_lock(
            &mutex
        );


        /*
         * Shared Resource
         */

        shared_resource++;


        /*
         * Unlock
         */

        kernel_mutex_unlock(
            &mutex
        );


        task_delay(600);
    }
}


/* =========================================================
 * MAIN
 * ========================================================= */

int main(void)
{
    /* -----------------------------------------------------
     * GPIO
     * ----------------------------------------------------- */

    GPIO_Init();


    /* -----------------------------------------------------
     * Kernel
     * ----------------------------------------------------- */

    kernel_init();


    /* -----------------------------------------------------
     * Queue
     *
     * Element = uint32_t
     * Queue size = 8
     * ----------------------------------------------------- */

    kernel_queue_init(
        &queue,
        sizeof(uint32_t),
        8
    );


    /* -----------------------------------------------------
     * Semaphore
     *
     * Binary semaphore
     * ----------------------------------------------------- */

    kernel_sem_init(
        &semaphore,
        1
    );


    /* -----------------------------------------------------
     * Mutex
     * ----------------------------------------------------- */

    kernel_mutex_init(
        &mutex
    );


    /* -----------------------------------------------------
     * Producer Task
     * ----------------------------------------------------- */

    kernel_task_create(
        Producer_Task,
        NULL,
        1,
        256,
        0
    );


    /* -----------------------------------------------------
     * Consumer Task
     * ----------------------------------------------------- */

    kernel_task_create(
        Consumer_Task,
        NULL,
        1,
        256,
        0
    );


    /* -----------------------------------------------------
     * Semaphore Task
     * ----------------------------------------------------- */

    kernel_task_create(
        Semaphore_Task,
        NULL,
        1,
        256,
        0
    );


    /* -----------------------------------------------------
     * Mutex Task A
     * ----------------------------------------------------- */

    kernel_task_create(
        Mutex_Task_A,
        NULL,
        1,
        256,
        0
    );


    /* -----------------------------------------------------
     * Mutex Task B
     * ----------------------------------------------------- */

    kernel_task_create(
        Mutex_Task_B,
        NULL,
        1,
        256,
        0
    );


    /* -----------------------------------------------------
     * START KERNEL
     * ----------------------------------------------------- */

    kernel_start();


    /* -----------------------------------------------------
     * Should never reach here
     * ----------------------------------------------------- */

    while (1)
    {
    }


    return 0;
}
