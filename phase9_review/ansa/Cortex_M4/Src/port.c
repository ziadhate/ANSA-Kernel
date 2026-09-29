#include "port.h"
#include "scheduler.h"

#define SCB_ICSR        (*(volatile uint32_t *)(0xE000ED04UL))
#define ICSR_PENDSVSET  (1UL << 28)

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
    SysTick_CTRL = (1 << 2)   /* CLKSOURCE = CPU clock */
                 | (1 << 1)   /* SysTick exception request enable */
                 | (1 << 0);  /* Counter enable */

    /* PendSV priority set to the lowest (0xFF) */
    SCB_SHPR3 |= (0xFF << 16);
}

void port_start_first_task(void)
{
    /* Reset PSP to 0 so PendSV knows there is no running task to save */
    __asm volatile ("movs r0, #0");
    __asm volatile ("msr psp, r0");

    critical_nesting = 0;

    SCB_ICSR = ICSR_PENDSVSET;
    __asm volatile ("cpsie i" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

uint32_t *port_stack_init(uint32_t *stack_top, void (*fn)(void *), void *arg) {
    /* Ensure 8-byte stack alignment */
    uint32_t *sp = (uint32_t *)((uint32_t)stack_top & ~7UL);

    /* Hardware exception frame */
    *(--sp) = 0x01000000UL;              /* xPSR: Thumb bit MUST be set */
    *(--sp) = (uint32_t)fn | 1UL;        /* PC: Force Thumb bit (LSB = 1) */
    *(--sp) = 0xFFFFFFFDUL;              /* LR: EXC_RETURN (Thread mode, PSP stack) */
    *(--sp) = 0x12121212UL;              /* R12 */
    *(--sp) = 0x03030303UL;              /* R3 */
    *(--sp) = 0x02020202UL;              /* R2 */
    *(--sp) = 0x01010101UL;              /* R1 */
    *(--sp) = (uint32_t)arg;             /* R0: Task parameter */

    /* Software saved registers (R4-R11) */
    for (int i = 0; i < 8; i++) {
        *(--sp) = 0x00000000UL;
    }

    return sp;
}

__attribute__((naked)) void PendSV_Handler(void)
{
    __asm volatile (
        "cpsid   i                          \n"
        "mrs     r0, psp                    \n"
        "cbz     r0, save_done              \n"

        "stmdb   r0!, {r4-r11}              \n" /* Save R4-R11 to previous task PSP */
        "ldr     r1, =current_task          \n"
        "ldr     r1, [r1]                   \n"
        "str     r0, [r1]                   \n" /* current_task->sp = r0 */

        "save_done:                         \n"
        "push    {lr}                       \n"
        "bl      scheduler_select_next_task \n"
        "pop     {lr}                       \n"

        "ldr     r1, =current_task          \n"
        "ldr     r2, =next_task             \n"
        "ldr     r2, [r2]                   \n"
        "str     r2, [r1]                   \n" /* current_task = next_task */

        "ldr     r0, [r2]                   \n" /* r0 = next_task->sp */
        "ldmia   r0!, {r4-r11}              \n" /* Restore R4-R11 */
        "msr     psp, r0                    \n"

        "ldr     lr, =0xFFFFFFFD            \n" /* Return to Thread mode on PSP */
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
    kernel_update_ticks();
}
