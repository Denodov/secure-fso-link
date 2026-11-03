#include "stm32f303x8.h"
#include "uart.h"
#include "clock.h"
#include "time.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_UART_BUF 1000

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
    char buf[MAX_UART_BUF];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, MAX_UART_BUF, format, args);
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
    USART2->CR1 = 0; // USART disabled, 8 bit, no parity, 16x oversampling 
    USART2->CR2 = 0; // 1 stop bit                   
    USART2->CR3 = 0; // no DMA

    /* For prototype, we will use 9.6kbps baudrate & oversampling by 16.
       Switch to higher baudrate and less sampling upon reliable link. 
       Also... add USART1 later for actual link data transfer
       and keep USART2 for USB. */
    USART2->BRR = (0x0D05); // Setting baud rate to 9600. 
                            // USART2 is on APB1, so clock freq. = 32MHz
                            // 32MHz / 9600 = ~3333 = 0D05 

    USART2->CR1 |= USART_CR1_TE; // Set TE bit to be enabled (transmitter enabled)
    USART2->CR1 |= USART_CR1_RE; // Set RE bit to be enabled (receiver enabled)

    // Set PA2 to alternate function so it transmits to the ST-Link
    GPIOA->MODER &= ~(3u << 4);  // set bits 4-5 to 00
    GPIOA->MODER |= (2u << 4);   // set bits 4-5  to 10 (alternate function)
    GPIOA->AFR[0] &= ~(0xFu << 8); // clear alternate function
    GPIOA->AFR[0] |= (7u << 8);  // set to alternate function 7
    GPIOA->OTYPER &= ~(1u << 2); // transmit push-pull 

    // Set PA15 to alternate function 7 so it receives
    GPIOA->MODER &= ~(3u << 30);      // set bits 30-31 to 00
    GPIOA->MODER |= (2u << 30);       // set bits 30-31 to 10 (alternate function)
    GPIOA->AFR[1] &= ~(0xFu << (28)); // clear alternate function
    GPIOA->AFR[1] |= (7u << (28));    // set to alternate function 7

    USART2->CR1 |= USART_CR1_UE; // Enable USART

    uart_enabled_f = 1;

//    NVIC_EnableIRQ(USART2_IRQn);
//    USART2->CR1 |= USART_CR1_RXNEIE; // enable receive interrupts
}

