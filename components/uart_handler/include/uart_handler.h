#ifndef UART_HANDLER
#define UART_HANDLER

#include "stdint.h"

#define MAX_LINE_LEN 64

void uart_init(void);
int uart_read(uint8_t *byte);
int uart_write(void *msg, size_t size);

#endif // !UART_HANDLER
