#include <stdint.h>

#include "kernel.h"
#include "task.h"


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


static void GPIO_Init(void)
{
    RCC_AHB1ENR |= GPIOAEN;

    /* PA5 Output */
    GPIOA_MODER &= ~(3U << (5 * 2));
    GPIOA_MODER |=  (1U << (5 * 2));
}


/* =========================================================
 * LED Task
 * ========================================================= */

static void LED_Task(void *arg)
{
    (void)arg;

    while (1)
    {
        GPIOA_ODR ^= PA5;

        task_delay(500);
    }
}


/* =========================================================
 * Main
 * ========================================================= */

int main(void)
{
    GPIO_Init();

    kernel_init();

    kernel_task_create(
        LED_Task,
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
