#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

#define DEBUG_UART1 1
#define DEBUG_UART2 2
#define DEBUG_UART3 3

#ifndef DEBUG
#define DEBUG DEBUG_UART1
#endif

void usart_printf_init(uint32_t baudrate);

#endif
