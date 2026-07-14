#include "scheduler.h"


#define SYSTICK_BASE        (0xE000E010UL)
#define SYSTICK_CTRL        (*(volatile uint32_t*)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD        (*(volatile uint32_t*)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL         (*(volatile uint32_t*)(SYSTICK_BASE + 0x08))

#define PERIPH_BASE_ADDR   (0x40000000UL)
#define AHB2_BASE_ADDR     (PERIPH_BASE_ADDR + 0x08000000UL)
#define GPIOA_BASE_ADDR    (AHB2_BASE_ADDR)
#define AHB1_BASE_ADDR     (PERIPH_BASE_ADDR + 0x00020000UL)
#define RCC_BASE_ADDR      (AHB1_BASE_ADDR + 0x1000UL)
#define RCC_AHB2ENR_ADDR   (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x4CUL))
#define GPIOA_MODER_ADDR   (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x00UL))
#define GPIOA_ODR_ADDR     (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x14UL))
#define GPIOAEN            (1U << 0)
#define LED_PIN            (1U << 5)

void blink_task(void) {
    GPIOA_ODR_ADDR ^= LED_PIN;
}

void SysTick_Handler(void) {
    tick_increment();
}

int main(void) {
    RCC_AHB2ENR_ADDR |= GPIOAEN;
    GPIOA_MODER_ADDR &= ~(0x3U << (5 * 2));
    GPIOA_MODER_ADDR |=  (0x1U << (5 * 2));

    SYSTICK_LOAD = (4000000UL / 1000U) - 1U;
    SYSTICK_VAL  = 0;
    SYSTICK_CTRL = 0x7; // ENABLE | TICKINT | CLKSOURCE

    scheduler_init();
    scheduler_set_mode(SCHED_COOPERATIVE);
    scheduler_set_injection_queue(0);
    create_task(blink_task, "blink", 500);
    // yhughj
    while (1) {
        scheduler_run();
    }
}