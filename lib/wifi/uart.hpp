#pragma once

#include <cstdint>

class Clock;

void uart_send(const char *str_p);
void uart_init(uint32_t baudrate, Clock& bus_clock);
int uart_putchar(char c);
int uart_getchar(void);
