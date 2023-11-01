#include "stm32f3xx.h"                // Device header

#ifndef __FUNC_UART_H__
#define __FUNC_UART_H__

void USART_config(uint32_t baudrate);
void USART_Send(char c);
void USART_putString(char * string);
void USART2_Enable_Pins(uint32_t baudrate);
void tokenizer(void);

#endif /* __FUNC_UART_H__ */
