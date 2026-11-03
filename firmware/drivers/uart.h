#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();

void uart_printf(char *format, ...);
void uart_print_byte(char b);
void uart_printstr(char *s); 
void uart_printnum(int16_t n); 

int uart_receive(char *buf, unsigned count, uint32_t timeout_ms);

#endif 
