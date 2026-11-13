#include "stm32f303x8.h"
#include "uart.h"
#include "clock.h"
#include "time.h"
#include "../src/ring_buffer.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_USART_BUF 1000
#define RX_CAPACITY 256

static uint8_t usb_rx_storage[RX_CAPACITY];
static uint8_t fso_rx_storage[RX_CAPACITY];

static rbd_t usb_rx;
static rbd_t fso_rx;

// count bytes dropped due to full buffer
static volatile uint32_t usb_dropped = 0;
static volatile uint32_t fso_dropped = 0;

void uart_printf(char *format, ...);

void uart1_write_byte(uint8_t byte) {
    while(!(USART1->ISR & USART_ISR_TXE)) {}
    USART1->TDR = byte;
}

void uart2_write_byte(uint8_t byte) {
    while (!(USART2->ISR & USART_ISR_TXE)) {}
    USART2->TDR = byte;
}

void uart_printstr(char *s) {
    char *curr = s;
    while (*curr != '\0') {
        uart2_write_byte(*curr);
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

// drain buffers
void uart_process(void) {
    uint8_t byte;

    if(USART1->ISR & USART_ISR_TXE) {
        if(ring_buffer_read(usb_rx, &byte) == 0) {
            USART1->TDR = byte;
        }
    }
     if(USART2->ISR & USART_ISR_TXE) {
        if(ring_buffer_read(fso_rx, &byte) == 0) {
            USART2->TDR = byte;
        }
    }
}

void USART1_IRQHandler(void) {
    if (USART1->ISR & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)USART1->RDR; // clears RXNE
            if(ring_buffer_write(fso_rx, &byte) != 0) {
                fso_dropped++;
            }
    }
    if (USART1->ISR & USART_ISR_ORE) {
        // clear overrun
        USART1->ICR = USART_ICR_ORECF;
    }
}
void USART2_IRQHandler(void) {
    if (USART2->ISR & USART_ISR_RXNE) {
        uint8_t byte = (uint8_t)USART2->RDR; // clears RXNE
            if(ring_buffer_write(usb_rx, &byte) != 0) {
                usb_dropped++;
            }
    }
    if (USART2->ISR & USART_ISR_ORE) {
        // clear overrun
        USART2->ICR = USART_ICR_ORECF;
    }
}

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

    /* === USART for FSO ===
     * match USB and FSO baudrates to simplify things.
     * set baudrate to 115200
     * USART1 is on APB2, so clock freq. = 64MHz
     * 64MHz/115200 = ~556 = 22C */
    USART1->BRR = (0x22c); 
    // USART1->BRR = (0x1A0B); // 9600

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
    
    // make USART1 be idle low so laser is off when idling
    USART1->CR2 |= USART_CR2_TXINV | USART_CR2_RXINV;

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

    rb_attr_t attr = {
        .s_elem = sizeof(uint8_t),
        .n_elem = RX_CAPACITY,
        .buffer = usb_rx_storage,
    };

    // Initialize USB ring buffer
    if(ring_buffer_init(&usb_rx, &attr) != 0) {
        while(true) {} // initialization failed 
    }

    attr.buffer = fso_rx_storage;

    // Initialize FSO ring buffer
    if(ring_buffer_init(&fso_rx, &attr) != 0) {
        while(true) {} // initialization failed
    }

    // enable interrupts
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_EnableIRQ(USART1_IRQn);
    USART2->CR1 |= USART_CR1_RXNEIE; 
    USART1->CR1 |= USART_CR1_RXNEIE;
}
