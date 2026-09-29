#include <stdint.h>

uint32_t SystemCoreClock = 8000000U;

/* Linker symbols */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/* Main */
extern int main(void);


/* ============================================================
 * Default Handler
 * ============================================================ */

void Default_Handler(void)
{
    while (1)
    {
    }
}


/* ============================================================
 * Reset Handler
 * ============================================================ */

void Reset_Handler(void)
{
    uint32_t *src;
    uint32_t *dst;

    /*
     * Copy initialized data
     * FLASH -> RAM
     */

    src = &_sidata;
    dst = &_sdata;

    while (dst < &_edata)
    {
        *dst++ = *src++;
    }


    /*
     * Clear BSS
     */

    dst = &_sbss;

    while (dst < &_ebss)
    {
        *dst++ = 0;
    }


    /*
     * Start application
     */

    main();


    while (1)
    {
    }
}


/* ============================================================
 * Interrupt Handlers
 * ============================================================ */

void NMI_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void HardFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void MemManage_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void BusFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void UsageFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void SVC_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void DebugMon_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void PendSV_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

void SysTick_Handler(void)
    __attribute__((weak, alias("Default_Handler")));


/* ============================================================
 * Vector Table
 * ============================================================ */

__attribute__((section(".isr_vector")))
const uintptr_t vector_table[] =
{
    (uintptr_t)&_estack,

    (uintptr_t)Reset_Handler,

    (uintptr_t)NMI_Handler,
    (uintptr_t)HardFault_Handler,
    (uintptr_t)MemManage_Handler,
    (uintptr_t)BusFault_Handler,
    (uintptr_t)UsageFault_Handler,

    0,
    0,
    0,
    0,

    (uintptr_t)SVC_Handler,
    (uintptr_t)DebugMon_Handler,

    0,

    (uintptr_t)PendSV_Handler,
    (uintptr_t)SysTick_Handler
};
