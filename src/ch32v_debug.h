#ifndef __DEBUG_H
#define __DEBUG_H

#include <ch32v10x.h>
#include <stdio.h>

#define ch32v10x_usb_hid 0

#define DEBUG_UART1 1
#define DEBUG_UART2 2
#define DEBUG_UART3 3

#ifndef DEBUG
#define DEBUG DEBUG_UART1
#endif

void Delay_Init(void);
void Delay_Us(uint32_t n);
void Delay_Ms(uint32_t n);
void USART_Printf_Init(uint32_t baudrate);

#endif

