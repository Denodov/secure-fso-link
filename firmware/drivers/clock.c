#include "clock.h"
#include "stm32f303x8.h"

int clock_init(void) {
    // enable HSI clock and wait until ready
    RCC->CR |= RCC_CR_HSION;          
    while(!(RCC->CR & RCC_CR_HSIRDY)) {}

    // make sure system clock is on HSI
    RCC->CFGR &= ~(RCC_CFGR_SW); // 00 = HSI 
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {}

    // disable PLL 
    RCC->CR &= ~(RCC_CR_PLLON);       
    while(RCC->CR & RCC_CR_PLLRDY) {}

    // allow two wait states for flash and enable prefetch 
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | 2u 
        | FLASH_ACR_PRFTBE;
    while((FLASH->ACR & FLASH_ACR_LATENCY) != 2u) {}
    while(!(FLASH->ACR & FLASH_ACR_PRFTBS)) {}

    /* 
     * HPRE  / 1 -> main clock divider
     * HSI   / 2 -> for PLL 
     * PPRE1 / 2 -> APB1 = 32MHz (max 36MHz) 
     * PPRE2 / 1 -> APB2 = 64MHz (max 72MHz)
    */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE)
        | RCC_CFGR_HPRE_DIV1;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLSRC)
        | RCC_CFGR_PLLSRC_HSI_DIV2;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1)
        | RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2) 
        | RCC_CFGR_PPRE2_DIV1;

    // multiply PLL input clock by 16
    // clock speed 8Mhz/2 * 16 = 64Mhz
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLMUL)
        | RCC_CFGR_PLLMUL16;          

    // enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY)) {}

    // switch system clock to PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW)
                | RCC_CFGR_SW_PLL; 
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}

    SystemCoreClockUpdate(); // system clock now 64MHz

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    // enable GPIO A
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;    // enable GPIO B
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // enable clock for UART peripheral
 
    return 0; 
}
