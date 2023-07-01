#pragma once

#include <cstdint>

typedef enum trans_e
{
    TRANS_OFF,
    TRANS_ON,
    TRANS_ENDING
} trans_t;

void spi_init(uint32_t spi_clock);
void spi_send(const char *str_p);
void spi_recv(char *str_p, uint32_t len);
trans_t spi_transparent(void);
