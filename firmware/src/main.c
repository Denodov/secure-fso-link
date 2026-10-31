#include "stm32f303x8.h"
#include <stdbool.h>
#include "../drivers/time.h"

static void system_init(void) {
    // Enable the clock on the GPIOA, GPIOB 
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN_Msk;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN_Msk;

    // Set GPIOB pins to output
    // PB3 is set on bits 6-7 
    GPIOB->MODER &= ~(3u << 6);   // set bits 6-7 to 00 (input/reset state)
    GPIOB->MODER |= (1u << 6);    // set bits 6-7 to 01 (output)
    GPIOB->OTYPER &= ~(1u << 3);   // set bit 3 to 0
    GPIOB->OSPEEDR &= ~(3u << 6); // set speed bits to 00

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);
}

int main(void) {
    system_init(); // Configures the clock to tick every 1ms
    while (true) {
        GPIOB->ODR ^= (1u << 3); // toggles PB3 (onboard LED) 
        delay(100);
    }
}
