#include "stm32f303x8.h"
#include "uart.h"
#include "clock.h"
#include "time.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_USART_BUF 1000

static int uart_enabled_f = 0;

void uart_printf(char *format, ...);

void uart_print_byte(char c) {
    if (!uart_enabled_f) {
    return;
    }

    while (!(USART2->ISR & USART_ISR_TXE)) {
    }
    USART2->TDR = c;
}

void uart_printstr(char *s) {
    char *curr = s;
    while (*curr != '\0') {
        uart_print_byte(*curr);
        curr++;
    }
}

void uart_printnum(int16_t n) {
    char buf[100];
    snprintf(buf, sizeof(buf), "%hd", n);
    uart_printstr(buf);
}

void uart_printf(char *format, ...) {
    char buf[MAX_USART_BUF];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, MAX_USART_BUF, format, args);
    va_end(args);

    uart_printstr(buf);
}
/*
int uart_receive(char *buf, unsigned count, uint32_t timeout_ms) {
    uint32_t start = millis();
    unsigned received = 0;

    while (received < count) {
        if ((uint32_t)(millis() - start) >= timeout_ms) {
            return -1; // timed out 
        }

        if (USART2->ISR & USART_ISR_ORE) {
            USART2->ICR = USART_ICR_ORECF;
        }

        if (USART2->ISR & USART_ISR_RXNE) {
            buf[received++] = (char)USART2->RDR;
        }
    }

    return 0; // received all bytes 
}

void USART2_IRQHandler(void) {
    if (USART2->ISR & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)USART2->RDR; // clears RXNE
        uart_print_byte((char)byte);
    }

    if (USART2->ISR & USART_ISR_ORE) {
        // clear overrun
        USART2->ICR = USART_ICR_ORECF;
    }
}
*/

void uart_init() {
    /* CR1 = 0: USART disabled, 8 bit, no parity, 16x oversampling
     * CR2 = 0: 1 stop bit
     * CR3 = 0: no DMA */
    USART1->CR1 = 0; 
    USART1->CR2 = 0;
    USART1->CR3 = 0; 

    USART2->CR1 = 0;
    USART2->CR2 = 0; 
    USART2->CR3 = 0;

    /* For prototype, we will use 9.6kbps baudrate & oversampling by 16.
       Switch to higher baudrate and less sampling upon reliable link.*/ 

    /* === USART for FSO ===
     * set baudrate to 9600
     * USART1 is on APB2, so clock freq. = 64MHz
     * 64MHz/9600 = ~6667 = 1A0B */
    USART1->BRR = (0x1A0B);

    /* === USART for USB ===
     * set baudrate to 115200
     * USART2 is on APB1, so clock freq. = 32MHz
     * 32MHz/115200 = ~278 = 116 */
    USART2->BRR = (0x116); 

    // Enable transmit/receive for USART1 & USART2 
    USART1->CR1 |= USART_CR1_TE; 
    USART1->CR1 |= USART_CR1_RE;
    USART2->CR1 |= USART_CR1_TE; 
    USART2->CR1 |= USART_CR1_RE; 
    
    // Set USART to alternate function so it transmits to the ST-Link
    /* "Each I/O pin has a multiplexer with up to sixteen alternate function inputs (AF0 to AF15) that
       can be configured through the GPIOx_AFRL (for pin 0 to 7) and GPIOx_AFRH (for pin 8 to 15) registers" */
    // USART1 (PA9/PA10), USART2 (PA2/PA3) or USB

    // USART1 transmit (PA9)
    GPIOA->MODER &= ~(3u << 18);   // clear bits 18-19 (MODER9) 
    GPIOA->MODER |= (2u << 18);    // set bits 18-19 to 10 (alternate function)
    GPIOA->AFR[1] &= ~(0xFu << 4); // clear bits 4-7 (AFR9)
    GPIOA->AFR[1] |= (7u << 4);    // set bits 4-7 to AF7 
    GPIOA->OTYPER &= ~(1u << 9);   // output push-pull 

    // USART1 receive (PA10)
    GPIOA->MODER &= ~(3u << 20);   // clear bits 20-21 (MODER10) 
    GPIOA->MODER |= (2u << 20);    // set bits 20-21 to 10 (alternate function)
    GPIOA->AFR[1] &= ~(0xFu << 8); // clear bits 8-11 (AFR10)
    GPIOA->AFR[1] |= (7u << 8);    // set bits 8-11 to AF7

    // ST-Link USB serial for USART2 is wired to PA2/PA15
    // USART2 transmit (PA2)
    GPIOA->MODER &= ~(3u << 4);    // set bits 4-5 (MODER2) to 00
    GPIOA->MODER |= (2u << 4);     // set bits 4-5  to 10 (alternate function)
    GPIOA->AFR[0] &= ~(0xFu << 8); // clear alternate function
    GPIOA->AFR[0] |= (7u << 8);    // set to alternate function 7
    GPIOA->OTYPER &= ~(1u << 2);   // output push-pull 

    // USART2 receive (PA15) note: PA3 is used for external USART2 connection 
    GPIOA->MODER &= ~(3u << 30);      // set bits 30-31 (MODER15) to 00
    GPIOA->MODER |= (2u << 30);       // set bits 30-31 to 10 (alternate function)
    GPIOA->AFR[1] &= ~(0xFu << (28)); // clear alternate function
    GPIOA->AFR[1] |= (7u << (28));    // set to alternate function 7

    // enable USART
    USART1->CR1 |= USART_CR1_UE;
    USART2->CR1 |= USART_CR1_UE; 

    uart_enabled_f = 1;

//    NVIC_EnableIRQ(USART2_IRQn);
//    USART2->CR1 |= USART_CR1_RXNEIE; // enable receive interrupts
}

