#include <stdint.h>

#include "task.h"
#include "scheduler.h"

/* ============================================================
 * STM32F103C4 GPIOA
 * ============================================================ */

#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x40010800UL

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18UL))

#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))

#define RCC_IOPAEN      (1U << 2)

/* ============================================================
 * LEDs
 * ============================================================ */

#define LED_A           (1U << 5)    /* PA5 */
#define LED_B           (1U << 6)    /* PA6 */
#define LED_C           (1U << 7)    /* PA7 */
#define LED_D           (1U << 8)    /* PA8 */

#define ALL_LEDS        (LED_A | LED_B | LED_C | LED_D)

/* ============================================================
 * Delay
 * ============================================================ */

static void delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm volatile ("nop");
    }
}

/* ============================================================
 * GPIO Initialization
 * ============================================================ */

static void GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC_APB2ENR |= RCC_IOPAEN;

    /*
     * PA5, PA6, PA7
     * Output 50 MHz Push-Pull
     */
    GPIOA_CRL &= ~(0xFFF00000UL);
    GPIOA_CRL |=  (0x33300000UL);

    /*
     * PA8
     * Output 50 MHz Push-Pull
     */
    GPIOA_CRH &= ~(0x0000000FUL);
    GPIOA_CRH |=  (0x00000003UL);

    /* All LEDs OFF */
    GPIOA_ODR &= ~ALL_LEDS;
}

/* ============================================================
 * Task A -> PA5
 * ============================================================ */

static void TaskA(void *arg)
{
    (void)arg;

    while (1)
    {
        GPIOA_ODR &= ~ALL_LEDS;
        GPIOA_ODR |= LED_A;

        delay(150000);

        kernel_yield();
    }
}

/* ============================================================
 * Task B -> PA6
 * ============================================================ */

static void TaskB(void *arg)
{
    (void)arg;

    while (1)
    {
        GPIOA_ODR &= ~ALL_LEDS;
        GPIOA_ODR |= LED_B;

        delay(150000);

        kernel_yield();
    }
}

/* ============================================================
 * Task C -> PA7
 * ============================================================ */

static void TaskC(void *arg)
{
    (void)arg;

    while (1)
    {
        GPIOA_ODR &= ~ALL_LEDS;
        GPIOA_ODR |= LED_C;

        delay(150000);

        kernel_yield();
    }
}

/* ============================================================
 * Task D -> PA8
 * ============================================================ */

static void TaskD(void *arg)
{
    (void)arg;

    while (1)
    {
        GPIOA_ODR &= ~ALL_LEDS;
        GPIOA_ODR |= LED_D;

        delay(150000);

        kernel_yield();
    }
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    /* Initialize GPIO */
    GPIO_Init();

    /* Initialize task pool */
    task_pool_init();

    /*
     * Four tasks
     *
     * Same priority = 1
     * Stack = 128 words
     * Period = 0
     */

    kernel_task_create(
        TaskA,
        NULL,
        1,
        128,
        0
    );

    kernel_task_create(
        TaskB,
        NULL,
        1,
        128,
        0
    );

    kernel_task_create(
        TaskC,
        NULL,
        1,
        128,
        0
    );

    kernel_task_create(
        TaskD,
        NULL,
        1,
        128,
        0
    );

    /* Start Kernel */
    kernel_start();

    /* Never reached */
    while (1)
    {
    }

    return 0;
}
