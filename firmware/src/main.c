#include "stm32f303x8.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../drivers/clock.h"
#include "../drivers/time.h"
#include "../drivers/uart.h"

static void system_init(void) {
    // PB3 is set on bits 6-7 
    GPIOB->MODER &= ~(3u << 6);   // set bits 6-7 to 00 (input/reset state)
    GPIOB->MODER |= (1u << 6);    // set bits 6-7 to 01 (output)
    GPIOB->OTYPER &= ~(1u << 3);  // set to push-pull
    GPIOB->OSPEEDR &= ~(3u << 6); // set to longest rise/fall time 

    SysTick_Config(SystemCoreClock / 1000u);
}

int main(void) {
    clock_init();
    system_init(); 
    uart_init();

    while(true) {
        uart_process();
    }
}
