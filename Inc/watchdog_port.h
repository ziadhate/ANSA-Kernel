#ifndef WATCHDOG_PORT_H
#define WATCHDOG_PORT_H

//This file i wrote specifically keeping in mind STM32L4 because Hardware watchdog reset is dependent on respective memory adresses 
//do the corresponding change to this addresses if compiling for any other board
//STM32L4 addresses i took from its memory map
//STM32L4 IWDG
//Base address: 0x40003000
#define IWDG_BASE       (0x40003000UL)
#define IWDG_KR         (*(volatile uint32_t *)(IWDG_BASE + 0x00UL))
#define IWDG_PR         (*(volatile uint32_t *)(IWDG_BASE + 0x04UL))
#define IWDG_RLR        (*(volatile uint32_t *)(IWDG_BASE + 0x08UL))
#define IWDG_SR         (*(volatile uint32_t *)(IWDG_BASE + 0x0CUL))

// IWDG key values.
#define IWDG_KEY_ENABLE     0xCCCCUL
#define IWDG_KEY_RELOAD     0xAAAAUL
#define IWDG_KEY_UNLOCK     0x5555UL

extern volatile uint32_t watchdog_debug_status;

//LSI is approximately 32 kHz.
//PR = 3 -> division by 32
//watchdog tick ≈ 1 kHz
#define IWDG_PRESCALER      3U

//Approximately 1 second timeout.
//  With:
//  LSI ≈ 32 kHz
//  prescaler = 32
//  watchdog clock ≈ 1 kHz
//  reload = 1000
//  timeout ≈ 1 second
#define IWDG_RELOAD_VALUE   10000U

void port_watchdog_start(void);
void port_watchdog_kick(void);

#endif /* WATCHDOG_PORT_H */