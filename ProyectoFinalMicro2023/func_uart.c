#include "func_uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char data;
uint8_t n = 0;
uint8_t cmd_ready = 0;
char tx_buffer[64];
char rx_buffer[64];

char * tokens[] = {" "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "};




/*void USART_config(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;						//Clock Enbale GPIOA
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					//Clock Enbale USART2

	GPIOA->MODER |= (0x02<<4) | (0x02<<30);				//Alternate Function PA2 & PA15
	GPIOA->AFR[0] |= (0x07<<8);									  //PA2 as TX2
	GPIOA->AFR[1] |= (0x07<<28);									//PA15 as RX2
	
	USART2->BRR = (uint32_t)(SystemCoreClock/baudrate);  //round( 8MHz/115200)
	USART2->CR1 |= USART_CR1_TE + USART_CR1_RE;					// Habiliar recepcion y transmision
	USART2->CR1 |= USART_CR1_RXNEIE;										// Interrupci�n recepcion
	USART2->CR1 |= USART_CR1_UE;												// Habilitar modulo UART (puerto serial)
	
	NVIC_EnableIRQ(USART2_IRQn);
}*/

void USART_config(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;						//Clock Enbale GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;					//Clock Enbale USART1

	GPIOA->MODER |= (0x02<<18) | (0x02<<20);				//Alternate Function PA9 & PA10
	GPIOA->AFR[1] |= (0x07<<4) | (0x07<<8);					//PA9 & pa10 as RX2
	
	USART1->BRR = (uint32_t)(SystemCoreClock/baudrate);  //round( 8MHz/115200)
	USART1->CR1 |= USART_CR1_TE + USART_CR1_RE;					// Habiliar recepcion y transmision
	USART1->CR1 |= USART_CR1_RXNEIE;										// Interrupci�n recepcion
	USART1->CR1 |= USART_CR1_UE;												// Habilitar modulo UART (puerto serial)
	
	NVIC_EnableIRQ(USART1_IRQn);
}



void USART1_IRQHandler(void){
	if(USART1->ISR & USART_ISR_RXNE){
		data = USART1->RDR;
		if(data != '\r'){
			rx_buffer[n] = data;
			n++;
			cmd_ready = 0;
		}else{
			rx_buffer[n] = '\0';
			n = 0;
			cmd_ready = 1;
		}
	}
}



void USART_Send(char c){
	while(!(USART1->ISR & USART_ISR_TC));
	USART1->TDR = c;
}

void USART_putString(char * string){
	while(*string){
		USART_Send(*string);
		string++;
	}
}

void tokenizer(void){
	tokens[0] = strtok(rx_buffer, " ");
	tokens[1] = strtok(NULL, " ");
	tokens[2] = strtok(NULL, " ");
	tokens[3] = strtok(NULL, " ");
	tokens[4] = strtok(NULL, " ");
	tokens[5] = strtok(NULL, " ");
	tokens[6] = strtok(NULL, " ");
	tokens[7] = strtok(NULL, " ");
	tokens[8] = strtok(NULL, " ");
	tokens[9] = strtok(NULL, " ");
	tokens[10] = strtok(NULL, " ");
	tokens[11] = strtok(NULL, " ");
	tokens[12] = strtok(NULL, " ");
}
