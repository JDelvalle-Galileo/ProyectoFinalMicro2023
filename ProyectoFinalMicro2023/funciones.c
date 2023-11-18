#include "funciones.h"
#include "func_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern void registerDisplayAssembly(uint32_t reg[]);
extern void registerDisplayAssembly(uint32_t reg[]);
extern void registerModifyAssemblyR6(uint32_t value);
extern void registerModifyAssemblyR7(uint32_t value);
extern void registerModifyAssemblyR8(uint32_t value);
extern void registerModifyAssemblyR9(uint32_t value);




//para BUZZER
static float duty_cycle;
static float periodo;
static float freq;
static uint32_t dc;
void PWM_SetFrequency(float freq_hz);
void PWM_SetPeriod(float period_ms);
void PWM_SetDutyCycle(float dc_ms);


//Para Register Display y Modify
static uint32_t registros[16]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint8_t reg_num;
static uint32_t reg_val;

//Para Memory Display y Modify
static uint8_t * memoryStart;
static uint8_t * memoryEnd;
static uint32_t * memoryModifyAddress;
static uint64_t i = 0;

//Para transmitir datos a terminal USART
extern char * tokens[];
extern char tx_buffer[64];



/* Function Definition*/


/*REGISTERS*/
void registerDisplay(void){
	registerDisplayAssembly(registros);
	USART_putString("Register Display! \r\n");
	for(i=0;i<16;i++){
		sprintf(tx_buffer, "R%d = %#010x \r\n",(int)i,registros[i]);
		USART_putString(tx_buffer);
	}
}
void registerModify(void){
	USART_putString("Register Modify! \r\n");
	reg_num = (uint8_t)strtol(&tokens[1][1], NULL, 16);
	reg_val = (uint32_t)strtoul(&tokens[2][0],NULL,16);
	if((reg_num>=6)&&(reg_num<=9)){
		if(reg_num == 6){registerModifyAssemblyR6(reg_val); USART_putString("Modify de 6\r\n");}
		if(reg_num == 7){registerModifyAssemblyR7(reg_val);USART_putString("Modify de 7\r\n");}
		if(reg_num == 8){registerModifyAssemblyR8(reg_val);USART_putString("Modify de 8\r\n");}
		if(reg_num == 9){registerModifyAssemblyR9(reg_val);USART_putString("Modify de 9\r\n");}
		sprintf(tx_buffer, "Se modifico R%d <= %#010x \r\n", reg_num, reg_val);
		USART_putString(tx_buffer);
		registerDisplayAssembly(registros);
		for(i=0;i<16;i++){
			if(i>5 && i<10){
			  sprintf(tx_buffer, "R%d = %#010x \r\n",(int)i,registros[i]);
		    USART_putString(tx_buffer);
			}
		}
	}else{
		USART_putString("Solo puede modificar registros 6 a 9! \r\n");
	}
}




/*MEMORIA*/
void memoryDisplay(void){
	if(tokens[1]==0x00000000 || tokens[2]==0x00000000){
		sprintf(tx_buffer,"Faltan parametros!, MemoryDisplay de 0x00000000: \r\n");
		USART_putString(tx_buffer);
		memoryStart = (uint32_t *)strtol("", NULL, 16);
		memoryEnd = (uint32_t *)strtol("", NULL, 16);
	}else{
		sprintf(tx_buffer,"MemoryDisplay de %s a %s: \r\n",tokens[1],tokens[2]);
		USART_putString(tx_buffer);
		memoryStart = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
		memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
	}
	//memoryStart = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
	//memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
	while(memoryStart <= memoryEnd){
		sprintf(tx_buffer, "%#02x ", *memoryStart);
		USART_putString(tx_buffer);
		memoryStart++;
	}
	USART_putString("\r\n");
}
/*void memoryModify(void){
	sprintf(tx_buffer,"Memory Modify\r\n");
	USART_putString(tx_buffer);
	memoryModifyAddress = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
	*memoryModifyAddress = (uint32_t)strtoul(&tokens[2][0], NULL, 16);
}*/
void memoryModify(void){
	USART_putString("Memory Modify! \r\n");
	if(tokens[1]==0x00000000 || tokens[2]==0x00000000){
		sprintf(tx_buffer,"Faltan parametros! \r\n");
		USART_putString(tx_buffer);
	}else{
		int size = 1;
		memoryModifyAddress = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
		if(!(tokens[3]==0x00000000)){
			size = strtol(&tokens[3][0],NULL, 10);
		}
		sprintf(tx_buffer,"MemoryModify de %s <= %s con size %d \r\n",tokens[1],tokens[2],size);
		USART_putString(tx_buffer);
		
		
		if(size == 1){
			uint8_t value = (uint8_t)strtoul(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#04x \r\n", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else if(size == 2){
			uint16_t value = (uint16_t)strtoul(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#06x \r\n", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else if(size == 4){
			uint32_t value = (uint32_t)strtoul(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#010x \r\n", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else{
			sprintf(tx_buffer,"Solo puede usar 1, 2 o 4 como size! \r\n");
			USART_putString(tx_buffer);
		}
	}
}
void blockFill(void){
	sprintf(tx_buffer,"BF de %s a %s, valor %s: \r\n",tokens[1],tokens[2],tokens[3]);
	USART_putString(tx_buffer);
	USART_putString("Block Fill! \r\n");
	if(tokens[1]==0x00000000 || tokens[2]==0x00000000){
		sprintf(tx_buffer,"Faltan parametros!\r\n");
		USART_putString(tx_buffer);
	}else{
		sprintf(tx_buffer,"Block Fill de %s a %s, valor %s: \r\n",tokens[1],tokens[2],tokens[3]);
		USART_putString(tx_buffer);
		memoryStart = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
		memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
		
		while(memoryStart <= memoryEnd){
			*memoryStart = (uint32_t)strtol(&tokens[3][0], NULL, 16);
			sprintf(tx_buffer,"%#010x ",*memoryStart);
			USART_putString(tx_buffer);
			memoryStart = memoryStart+4;
		}
		USART_putString("\r\n");
	}
	//memoryStart = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
	//memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
	
}






/*FUNCIONES*/
void run(void){
	USART_putString("Run! \r\n");
}
void call(void){
	USART_putString("Call! \r\n");
}
void ioMap(void){
	USART_putString("I/O Map! \r\n");
}
void ioUnmap(void){
	USART_putString("I/O Unmap! \r\n");
}
void segmentOut(void){
	USART_putString("Segment Out! \r\n");
}
void lcdPrint(void){
	USART_putString("LCD Print! \r\n");
}
void sound(void){
	sprintf(tx_buffer,"Sound frecuencia %sHz \r\n",tokens[1]);
	USART_putString(tx_buffer);
	freq = (float)atof(&tokens[1][0]);

	PWM_SetFrequency(freq);
}
void mute(void){
	sprintf(tx_buffer,"Mute de Buzzer \r\n",tokens[1]);
	USART_putString(tx_buffer);
	PWM_SetDutyCycle(0); 
}

void PWM_SetFrequency(float freq_hz){
	dc = (2000000/freq_hz)-1;
	TIM16->ARR = (4000000/freq_hz)-1;
	TIM16->CCR1 = dc;
}

void PWM_SetDutyCycle(float dc_ms){
	dc = (SystemCoreClock/64)/((uint32_t)(1000/dc_ms));
	TIM16->CCR1 = dc;
}
