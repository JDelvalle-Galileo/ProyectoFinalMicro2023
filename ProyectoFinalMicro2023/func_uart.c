#include "func_uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char data;
uint8_t n = 0;
uint8_t cmd_ready = 0;
char tx_buffer[64];
char rx_buffer[32];

char * tokens[] = {" "," "," "," "};


void USART_command_read(){
	
			
}



void USART_config(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;						//Clock Enbale GPIOA
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					//Clock Enbale USART2

	GPIOA->MODER |= (0x02<<4) | (0x02<<30);				//Alternate Function PA2 & PA15
	GPIOA->AFR[0] |= (0x07<<8);									  //PA2 as TX2
	GPIOA->AFR[1] |= (0x07<<28);									//PA15 as RX2
	
	USART2->BRR = (uint32_t)(SystemCoreClock/baudrate);  //round( 8MHz/115200)
	USART2->CR1 |= USART_CR1_TE + USART_CR1_RE;					// Habiliar recepcion y transmision
	USART2->CR1 |= USART_CR1_RXNEIE;										// Interrupción recepcion
	USART2->CR1 |= USART_CR1_UE;												// Habilitar modulo UART (puerto serial)
	
	NVIC_EnableIRQ(USART2_IRQn);
}


//PA2 USART2_TX y PA15 USART2_RX		//Ambos son funcion 7
void USART2_Enable_Pins(uint32_t baudrate){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	//Habilitamos reloj a GPIOA
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //Clock Enbale USART2
	
	GPIOA->MODER |= (2<<4)|(2<<30); //Funci?n alternativa del pin 2 y 15
	GPIOA->AFR[0] |= (7<<8); //Funccion alternativa 7 del pin 2 
	GPIOA->AFR[1] |= (7<<28); //Funccion alternativa 7 del pin 15 
	
	USART2->BRR = (uint32_t)(SystemCoreClock/baudrate);  //round( 64MHz/115200)
	USART2->CR1 |= USART_CR1_TE + USART_CR1_RE;					// Habiliar recepcion y transmision
	USART2->CR1 |= USART_CR1_RXNEIE;										// Interrupcion recepcion
	USART2->CR1 |= USART_CR1_UE;												// Habilitar modulo UART (puerto serial)
	
	NVIC_EnableIRQ(USART2_IRQn);
}


void USART2_IRQHandler(void){
	if(USART2->ISR & USART_ISR_RXNE){
		data = USART2->RDR;
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
	while(!(USART2->ISR & USART_ISR_TC));
	USART2->TDR = c;
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
}
