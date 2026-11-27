#include "stm32f303x8.h"

/* MCP6021 output   -> noninverting COMP2 input (PA7)
 * Threshold (DAC) -> inverting COMP2 input
 * COMP2_OUT pin   -> external jumper -> USART1_RX (PA10) */

void comp_init() {
    // GPIOs
    // COMP2 non-inverting input (PA7)
    GPIOA->MODER &= ~(3u << (7*2));   // clear mode
    GPIOA->MODER |= (3u << (7*2));    // set to analog mode
    GPIOA->PUPDR &= ~(3u << (7*2));   // no pull-up, pull-down

    // COMP2 inverting input (DAC)
    // inverting input selection bits are split up for some reason: 22, 6, 5, 4 
    // DAC2_CH1 option is 1000, so 1u goes to bit 22 the rest are cleared
    COMP2->CSR &= ~(COMP2_CSR_COMP2INSEL); // clear inverting input selection
    COMP2->CSR |= (1u << 22);              // set to 1000 (DAC2_CH1)

    // COMP2 output (PA12)
    GPIOA->MODER &= ~(3u << (12*2));   // clear PA12 mode
    GPIOA->MODER |= (2u << (12*2));    // set to alternate function mode
    GPIOA->AFR[1] &= ~(0xFu << 16);    // clear PA12 AF
    GPIOA->AFR[1] |= (8u << 16);       // set to comp output function
    GPIOA->OTYPER &= ~(1u << 12);      // output push-pull
    GPIOA->OSPEEDR &= ~(3u << (12*2)); // clear 
    GPIOA->OSPEEDR |= (3u << (12*2));  // set to high speed 
    GPIOA->PUPDR &= ~(3u << (12*2));   // no pull-up, pull-down
    
    // invert if normal polarity gives idle high in darkness
    COMP2-> CSR &= ~COMP2_CSR_COMP2POL; // normal polarity
//  COMP2->CSR |= COMP2_CSR_COMP2POL;   // inverted polarity to match USART

    COMP2->CSR |= COMP2_CSR_COMP2EN; // enable comparator
}
