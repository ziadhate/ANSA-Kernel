#include "port.h"
#include "scheduler.h"   /* current_task, next_task, scheduler_switch_context(),
                             kernel_update_ticks() — the portable side */

// Those don't need to be visible outside
#define SCB_ICSR        (*(volatile uint32_t *)(0xE000ED04UL))
#define ICSR_PENDSVSET  (1UL << 28)

/*
	Maintenue par system_stm32l4xx.c / SystemCoreClockUpdate().
	Reflète toujours la fréquence système réelle (post-PLL), peu
	importe la config d'horloge choisie dans SystemClock_Config().
*/
extern uint32_t SystemCoreClock;

static volatile uint32_t critical_nesting = 0;

void port_critical_enter(void)
{
    __asm volatile ("cpsid i" ::: "memory");
    critical_nesting++;
}

void port_critical_exit(void)
{
    if (critical_nesting > 0U) {
        critical_nesting--;
    }
    if (critical_nesting == 0U) {
        __asm volatile ("cpsie i" ::: "memory");
    }
}

void port_trigger_context_switch(void)
{
    SCB_ICSR = ICSR_PENDSVSET;
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

void port_systick_init(uint32_t tick_rate_hz)
{
    SysTick_LOAD = (SystemCoreClock / tick_rate_hz) - 1;
    SysTick_VAL  = 0;
    SysTick_CTRL = (1 << 2)   /* CLKSOURCE */
                 | (1 << 1)   /* SysTick interrupt */
                 | (1 << 0);  /* ENABLE */

    /* PendSV at the lowest priority */
    SCB_SHPR3 |= (0xFF << 16);
}

void port_start_first_task(void)
{
    __asm volatile ("movs r0, #0");
    __asm volatile ("msr psp, r0");

    SCB_ICSR = ICSR_PENDSVSET;
    __asm volatile ("cpsie i" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

uint32_t *port_stack_init(uint32_t *stack_top, void (*fn)(void *), void *arg) {
    uint32_t *sp = stack_top;
 
    *(--sp) = 0x01000000UL;   // xPSR: thumb bit set  
    *(--sp) = (uint32_t)fn;   // PC: task entry point                         
    *(--sp) = 0xFFFFFFFDUL;   // LR: EXC_RETURN     
    *(--sp) = 0x12121212UL;   // R12                                          
    *(--sp) = 0x03030303UL;   // R3                                           
    *(--sp) = 0x02020202UL;   // R2                                           
    *(--sp) = 0x01010101UL;   // R1                                           
    *(--sp) = (uint32_t)arg;  // R0: first argument to fn                     
 
    for (int i = 0; i < 8; i++) {
        *(--sp) = 0x00000000UL; // R4-R1
    }
 
    return sp;
}


/* --- Interrupt handlers: hardware entry points, must stay here --- */

__attribute__((naked)) void PendSV_Handler(void)
{
    __asm volatile (
        "cpsid   i                          \n"
        "mrs     r0, psp                    \n"
        "cbz     r0, save_done              \n"

        "stmdb   r0!, {r4-r11}              \n" /* save R4-R11 to old task's stack */
        "ldr     r1, =current_task          \n"
        "ldr     r1, [r1]                   \n"
        "str     r0, [r1]                   \n" /* current_task->sp = r0 (sp is 1st member) */

        "save_done:                         \n"
        "push    {lr}                       \n"
        "bl      scheduler_select_next_task   \n"
        "pop     {lr}                       \n"

        "ldr     r1, =current_task          \n"
        "ldr     r2, =next_task             \n"
        "ldr     r2, [r2]                   \n"
        "str     r2, [r1]                   \n" /* current_task = next_task        */

        "ldr     r0, [r2]                   \n" /* r0 = next_task->sp              */
        "ldmia   r0!, {r4-r11}              \n" /* restore R4-R11 for the new task */
        "msr     psp, r0                    \n"

        "ldr     lr, =0xFFFFFFFD            \n" /* force: Thread mode, use PSP     */
        "cpsie   i                          \n"
        "bx      lr                         \n"
    );
}

void port_idle(void)
{
    __asm volatile ("wfi");
}

void SysTick_Handler(void)
{
    kernel_update_ticks();   // g_tick_count++ now lives in scheduler.c, exposed via kernel_get_tick_count()
}
