#include "funciones.h"
#include "func_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Para Register Display y Modify
static uint32_t registros[16]  = {999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Para Memory Display y Modify
static uint8_t * memoryStart;
static uint8_t * memoryEnd;
static uint8_t * memoryModifyAddress;

//Para transmitir datos a terminal USART
extern char * tokens[];
extern char tx_buffer[64];



/* Function Definition*/

void registerDisplay(void){
	USART_putString("Register Display! \r\n");
	int i = 0;
	for(i=0;i<16;i++){
		sprintf(tx_buffer, "R%d = %#010x\n", i, registros[i]);
		USART_putString(tx_buffer);
	}
}
void registerModify(void){
	USART_putString("Register Modify! \r\n");
}
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
		sprintf(tx_buffer, "%#010x ", *memoryStart);
		USART_putString(tx_buffer);
		memoryStart++;
	}
	USART_putString("\r\n");
}




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
			uint8_t value = (uint8_t)strtol(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = (uint32_t)value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#04x ", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else if(size == 2){
			uint16_t value = (uint16_t)strtol(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#06x ", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else if(size == 4){
			uint32_t value = (uint32_t)strtol(&tokens[2][0], NULL, 16);
			*memoryModifyAddress = value;
			sprintf(tx_buffer, "Se modifico %#010x <= %#010x ", memoryModifyAddress, value);
			USART_putString(tx_buffer);
		}else{
			sprintf(tx_buffer,"Solo puede usar 1, 2 o 4 como size! \r\n");
			USART_putString(tx_buffer);
		}
	}
}
void blockFill(void){
	USART_putString("Block Fill! \r\n");
}
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
