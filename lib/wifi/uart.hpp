#pragma once

#include <cstdint>

void uart_send(const char *str_p);
void uart_init(uint32_t baudrate);
int uart_putchar(char c);
int uart_getchar(void);
