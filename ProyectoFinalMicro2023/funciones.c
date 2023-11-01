#include "funciones.h"
#include "func_uart.h"
#include <stdio.h>

/* Function Definition*/

void registerDisplay(void){
	USART_putString("Register Display! \r\n");
}
void registerModify(void){
	USART_putString("Register Modify! \r\n");
}
void memoryDisplay(void){
	USART_putString("Memory Display! \r\n");
}
void memoryModify(void){
	USART_putString("Memory Modify! \r\n");
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
