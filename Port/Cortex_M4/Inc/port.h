#ifndef PORT_H
#define PORT_H

#include <stdint.h>

/* --- Register map, lives here now instead of scheduler.h --- */
#define SysTick_BASE    (0xE000E010UL)
#define SysTick_CTRL    (*(volatile uint32_t *)(SysTick_BASE + 0x00))
#define SysTick_LOAD    (*(volatile uint32_t *)(SysTick_BASE + 0x04))
#define SysTick_VAL     (*(volatile uint32_t *)(SysTick_BASE + 0x08))

#define SCB_SHPR3       (*(volatile uint32_t *)(0xE000ED20UL))

/*
 * Hardware abstraction interface. scheduler.c must only call these
 * functions — no register access, no inline asm, no vendor macros.
 * A different MCU port implements the same interface in its own
 * Port/<MCU>/Src/port.c without touching Kernel/.
 */

/* --- Critical sections (was CRITICAL_SECTION_ENTER/EXIT) --- */
void port_critical_enter(void);
void port_critical_exit(void);

/* --- Context switch trigger (was SCB_ICSR |= ICSR_PENDSVSET) --- */
void port_trigger_context_switch(void);

/* --- Tick timer setup (was the SysTick_* register writes in kernel_init) --- */
void port_systick_init(uint32_t tick_rate_hz);

/* --- Kernel entry (was the raw psp=0 / cpsie / first PendSV in kernel_start) --- */
void port_start_first_task(void);

uint32_t *port_stack_init(uint32_t *stack_top, void (*fn)(void *), void *arg);

/*
 * TCB-level context switch. PSP save/restore and R4-R11 stacking stay
 * naked asm — that can't be abstracted away, it IS the hardware. This
 * declares the handler that must be wired into the STM32 interrupt
 * vector table (startup_stm32l476.s) for PendSV. scheduler_switch_context()
 * (in scheduler.c, portable) is called from inside it.
 */
void PendSV_Handler(void);
void SysTick_Handler(void);
void port_idle(void);

#endif
