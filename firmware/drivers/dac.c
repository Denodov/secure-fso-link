#include "stm32f303x8.h"

void dac_init() {
    /* DAC output = VREF * DAC value / 4096
     * Required DAC output: 1.65V -> DAC value = 2048*/

    DAC2->CR = 0;                             // clear config
    DAC2->DHR12R1 &= ~(DAC_DHR12R1_DACC1DHR); // clear data register
    DAC2->DHR12R1 |= (2048u);                 // DAC value = 2048
    DAC2->CR |= DAC_CR_EN1;                   // enable DAC2
}
