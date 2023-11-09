#include "stm32f3xx.h"          // Device header
#include "funciones.h"					// Funciones a utilizar
#include "func_uart.h"					//funciones de UART

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void clk_config(void);
void PWM_Init(void);
void IC_Init(void);
void read_command(void);


static float period_us = 0.0f;
static float ap_us = 0.0f;
static float freq_khz = 0.0f;
static float duty_cycle = 0.0f; // 0-100%
static uint16_t capture_rise = 0;
static uint16_t capture_rise_ant = 0;
static uint16_t capture_fall = 0;
static uint32_t count = 0;

extern char data;
extern uint8_t n;
extern uint8_t cmd_ready;
extern char tx_buffer[64];
extern char rx_buffer[32];

extern char * tokens[];




int main(void){
	
	clk_config();
	PWM_Init();
	IC_Init();

	
	USART_config(115200*2);

	USART_putString("\n\r-------------------------------------------------------------------------\n\r");
	USART_putString("               Proyecto Final Microprocesadores 2023       \n\r");
	USART_putString("       Julio Del Valle - 21000492, Jose Luis Díaz - 21000111                 \n\r");
	USART_putString("          Diego Ventura - 20000164, Jose Ortiz - 20006195                    \n\r");
	USART_putString(">> ");
	
	while(1){
		freq_khz = (1.0f/period_us)*1000.0f;
		duty_cycle = ap_us/period_us;
		read_command();
	}
}



void clk_config(void){
	// PLLMUL <- 0x0E (PLL input clock x 16 --> (8 MHz / 2) * 16 = 64 MHz )  
	RCC->CFGR |= 0xE<<18;
	// Flash Latency, two wait states for 48<HCLK<=72 MHz
	FLASH->ACR |= 0x2;
	// PLLON <- 0x1 
  RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));	
	// SW<-0x02 (PLL as System Clock), HCLK not divided, PPRE1<-0x4 (APB1 <- HCLK/2), APB2 not divided 
	RCC->CFGR |= 0x402;
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	SystemCoreClockUpdate();	
}

void TIM3_IRQHandler(void){
		count++;
		if(TIM3->SR & TIM_SR_CC3IF){	//ocurrio una captura en IC3 (flanco de subida)
			capture_rise = TIM3->CCR3;
			if((capture_rise-capture_rise_ant>0))
				period_us = (capture_rise-capture_rise_ant)*15.6e-3; //
			capture_rise_ant = capture_rise;
		}
		if(TIM3->SR & TIM_SR_CC4IF){  //ocurrio una captura en IC4 (flanco de bajada)
			capture_fall = TIM3->CCR4;
			if((capture_fall-capture_rise_ant)>0)
				ap_us = (capture_fall-capture_rise_ant)*15.6e-3;
		}
}

// PB4 - TIM16CH1 - AF4
void PWM_Init(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //Habilitar reloj a GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;	//Habilitar reloj a TIM16
	
	GPIOB->MODER |= (2<<8);  //Funcion alternativa del pin 4
	GPIOB->AFR[0] |= (1<<16);   //Funcion alternativa 1 del pin 4 (bit 16 del AFRL)
	
	TIM16->PSC = 15; // prescala de 16, fclk = 4 MHz
	TIM16->ARR = 400; // 4MHz/10kHz = 400 cuentas, frecuencia de pwm 10 kHz
	TIM16->CCR1 = 200;	// ancho de pulso (duty cycle 50%) // Es en CCR1 porque es Channel 1 en TIM16CH1
	// PWM mode 1 channel 3, Preload Enable
	TIM16->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1| TIM_CCMR1_OC1PE;//CCMR1 porque usamos canal 1 o 2. CCMR2 es para 3 y 4
	//ES LO MISMO QUE ;TIM16->CCMR1 |= (0x6<<4) + (1<<3);  //PWM Mode 1 (clear output on compare), Preload Enable
	
	// Autoreload, Counter enable, Edge Aligned, Upcounter 
	TIM16->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
	// Initialize preload registers
	TIM16->EGR |= TIM_EGR_UG;
	// Active High Polarity and Output Enable
	TIM16->CCER |= TIM_CCER_CC1E;
	// Main output enable
	TIM16->BDTR |= TIM_BDTR_MOE;
}


// PB0 - TIM3CH3 - AF2
void IC_Init(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	//Habilitamos reloj a GPIOB
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //Habilitamos reloj a TIM3
	
	GPIOB->MODER |= (2<<0); //Funci?n alternativa del pin 0
	GPIOB->AFR[0] |= (2<<0); //Funccion alternativa 2 del pin 0 (bit 0 del AFRL)
	
	/*// Prescaler (64MHz)/(63+1) = 1 MHz
	//TIM3->PSC = 0; 
	// Freq = 10kHz (9999) - Autoreload register in us 
	TIM3->ARR = 9999;
	// The update event is selected as trigger output (TRGO)
	TIM3->CR2 |= 0x20;
	// Update Generation, reinicializa el contador
	TIM3->EGR |= TIM_EGR_UG;*/
	
	TIM3->CCMR2 |= (1<<0) | (2<<8); //TI3 asociado a IC3 y a IC4
	TIM3->CCER &=  ~(TIM_CCER_CC3P + TIM_CCER_CC3NP); // IC3 capture in rising edge //coloca 00 para rising edge
	TIM3->CCER |= TIM_CCER_CC4P; //IC4 capture in falling edge			//coloca 01 para falling edge
	TIM3->CCER |= TIM_CCER_CC3E + TIM_CCER_CC4E; // Habiliar IC3 e IC4 
	TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; //Habilitar contador
	TIM3->DIER |= TIM_DIER_CC3IE + TIM_DIER_CC4IE; // habilitar interrupciones en IC3 e IC4	
	NVIC_EnableIRQ(TIM3_IRQn); // le avisamos a NVIC que vamos a usar las interrupciones de IC3 e IC4
	

}



void read_command(void){
    if(cmd_ready) {
        tokenizer();
			

            if(!strcmp(tokens[0],"RD")||!strcmp(tokens[0],"rd")){
                registerDisplay();
            }else if(!strcmp(tokens[0],"RM")||!strcmp(tokens[0],"rm")){
                registerModify();
            } else if(!strcmp(tokens[0],"MD")||!strcmp(tokens[0],"md")){
                memoryDisplay();
            } else if(!strcmp(tokens[0],"MM")||!strcmp(tokens[0],"mm")){
                memoryModify();
            } else if(!strcmp(tokens[0],"BF")||!strcmp(tokens[0],"bf")){
                blockFill();
            } else if(!strcmp(tokens[0],"RUN")||!strcmp(tokens[0],"run")){
                run();
            } else if(!strcmp(tokens[0],"CALL")||!strcmp(tokens[0],"call")){
                call();
            } else if(!strcmp(tokens[0],"IOMAP")||!strcmp(tokens[0],"iomap")){
                ioMap();
            } else if(!strcmp(tokens[0],"IOUNMAP")||!strcmp(tokens[0],"iounmap")){
                ioUnmap();
            } else if(!strcmp(tokens[0],"SEGMENT")||!strcmp(tokens[0],"segment")){
                segmentOut();
            } else if(!strcmp(tokens[0],"LCD")||!strcmp(tokens[0],"lcd")){
                lcdPrint();
            }else if(!strcmp(tokens[0],"clear")||!strcmp(tokens[0],"CLEAR")){
                USART_putString("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
            }else{
                USART_putString("¡Comando indefinido! \r\n");
            }

        USART_putString("\n>> ");
        cmd_ready = 0;
    } 
    


    /*
	if(cmd_ready){
				tokenizer();
			
				if(!strcmp(tokens[0],"RD")){
					//show_0();
					registerDisplay();
				}else if(!strcmp(tokens[0],"RM")){
					//show_1();
					//en tokens[1] está el registro a modificar
					//r_x = tokens[1];
					//register_modify(r_x[1] - '0');
					USART_putString("Register Modify! \r\n");
				}else if(!strcmp(tokens[0],"0")){
					sprintf(tx_buffer,"Periodo:  %.4f us\r\n", period_us);
					//respuesta = "Periodo: " period_us ""
					USART_putString(tx_buffer);
				}else if(!strcmp(tokens[0],"1")){
					sprintf(tx_buffer,"Frecuencia:  %.4f kHz\r\n", freq_khz);
					USART_putString(tx_buffer);
				}else if(!strcmp(tokens[0],"2")){
					sprintf(tx_buffer,"Ancho de pulso:  %.4f us\r\n", ap_us);
					USART_putString(tx_buffer);
				}else if(!strcmp(tokens[0],"3")){
					sprintf(tx_buffer,"Duty Cycle:  %.4f%% \r\n", duty_cycle*100);
					USART_putString(tx_buffer);
				}else if(!strcmp(tokens[0],"clear")){
					USART_putString("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
				}
				else{
					USART_putString("Comando indefinido! \r\n");
				}
				USART_putString("\n>> ");
				cmd_ready = 0;
			}*/
}




/*
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
*/
