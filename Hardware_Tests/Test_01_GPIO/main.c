#include <stdint.h>

/* ============================================================
 * STM32F103C4
 * PA5 Toggle Test
 * ============================================================ */

/* RCC */
#define RCC_BASE        0x40021000UL
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18UL))

/* GPIOA */
#define GPIOA_BASE      0x40010800UL
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))

#define IOPAEN          (1U << 2)
#define PA5             (1U << 5)


static void delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm volatile ("nop");
    }
}


static void GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC_APB2ENR |= IOPAEN;

    /*
     * PA5:
     *
     * MODE5 = 10 -> Output, 2 MHz
     * CNF5  = 00 -> General purpose push-pull
     */

    GPIOA_CRL &= ~(0xFU << 20);
    GPIOA_CRL |=  (0x2U << 20);
}


int main(void)
{
    GPIO_Init();

    while (1)
    {
        GPIOA_ODR ^= PA5;

        delay(500000);
    }

    return 0;
}
