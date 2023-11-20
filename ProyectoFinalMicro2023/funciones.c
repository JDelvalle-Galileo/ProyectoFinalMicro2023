#include "funciones.h"
#include "func_uart.h"
#include "Nokia5110.h"					//Funciones de pantalla Nokia
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define 	SLAVE_ADDR		0x40


extern void registerDisplayAssembly(uint32_t reg[]);
extern void registerDisplayAssembly(uint32_t reg[]);
extern void registerModifyAssemblyR6(uint32_t value);
extern void registerModifyAssemblyR7(uint32_t value);
extern void registerModifyAssemblyR8(uint32_t value);
extern void registerModifyAssemblyR9(uint32_t value);
extern void runAssembly(uint32_t * address);
extern void callAssembly(uint32_t * address);

void I2C_Write(char slave, char addr, char data);
void I2C_Read(char slave, char addr, char * data);




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
static uint32_t * memoryStart;
static uint32_t * memoryEnd;
static uint32_t * memoryModifyAddress;
static uint8_t * BFmemoryStart8;
static uint16_t * BFmemoryStart16;
static uint32_t * BFmemoryStart32;
static uint64_t i = 0;

//Para transmitir datos a terminal USART
extern char * tokens[];
extern char tx_buffer[64];


//paraLCD
static char lcd_buffer[70];

//para run/call
static uint32_t * jumpAddress;





/* -------------- Function Definition -----------------*/


/*para probar run y call*/
void printTest(void){
	USART_putString("Test Print \r\n");
}



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
		sprintf(tx_buffer, "%#010x ", *memoryStart);
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
	if(tokens[1]==0x00000000 || tokens[2]==0x00000000){
		sprintf(tx_buffer,"Faltan parametros!\r\n");
		USART_putString(tx_buffer);
	}else{
		if(tokens[4]==0x00000000) tokens[4] = "1";
		memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
		int size = strtol(&tokens[4][0],NULL, 10);
		
		sprintf(tx_buffer,"BF de %s a %s, valor %s, size %s \r\n",tokens[1],tokens[2],tokens[3],tokens[4]);
		USART_putString(tx_buffer);

		if(size == 1){
			//sprintf(tx_buffer,"Block Fill de %s a %s, valor %#04x: \r\n",tokens[1],tokens[2],(uint8_t)strtoul(&tokens[3][0], NULL, 16));
			//USART_putString(tx_buffer);
			uint8_t value = (uint8_t)strtoul(&tokens[3][0], NULL, 16);
			BFmemoryStart8 = (uint8_t *)strtol(&tokens[1][0], NULL, 16);
			while(BFmemoryStart8 <= memoryEnd){
				*BFmemoryStart8 = value;
				sprintf(tx_buffer,"%#04x ",*BFmemoryStart8);
				USART_putString(tx_buffer);
				BFmemoryStart8 = BFmemoryStart8+1;
			}
			USART_putString("\r\n");
		}else if(size == 2){
			//sprintf(tx_buffer,"Block Fill de %s a %s, valor %#06x: \r\n",tokens[1],tokens[2],(uint16_t)strtoul(&tokens[3][0], NULL, 16));
			//USART_putString(tx_buffer);
			uint16_t value = (uint16_t)strtoul(&tokens[3][0], NULL, 16);
			BFmemoryStart16 = (uint16_t *)strtol(&tokens[1][0], NULL, 16);
			while(BFmemoryStart16 <= memoryEnd){
				*BFmemoryStart16 = value;
				sprintf(tx_buffer,"%#06x ",*BFmemoryStart16);
				USART_putString(tx_buffer);
				BFmemoryStart16 = BFmemoryStart16+1;
			}
			USART_putString("\r\n");
		}else if(size == 4){
			//sprintf(tx_buffer,"Block Fill de %s a %s, valor %#010x: \r\n",tokens[1],tokens[2],(uint32_t)strtoul(&tokens[3][0], NULL, 16));
			//USART_putString(tx_buffer);
			uint32_t value = (uint32_t)strtoul(&tokens[3][0], NULL, 16);
			BFmemoryStart32 = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
			while(BFmemoryStart32 <= memoryEnd){
				*BFmemoryStart32 = value;
				sprintf(tx_buffer,"%#010x ",*BFmemoryStart32);
				USART_putString(tx_buffer);
				BFmemoryStart32 = BFmemoryStart32+1;
			}
			USART_putString("\r\n");
		}else{
			sprintf(tx_buffer, "Size solo puede ser 1, 2 o 4! Size actual %s \r\n", tokens[4]);
			USART_putString(tx_buffer);
		}


		
		/*while(memoryStart <= memoryEnd){
			*memoryStart = (uint8_t)strtoul(&tokens[3][0], NULL, 16);
			sprintf(tx_buffer,"%#04x ",*memoryStart);
			USART_putString(tx_buffer);
			memoryStart = memoryStart+1;
		}
		USART_putString("\r\n");*/
	}
	//memoryStart = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
	//memoryEnd = (uint32_t *)strtol(&tokens[2][0], NULL, 16);
	
}






/*LLAMADAS RUN Y CALL*/
void run(void){
	jumpAddress = (uint32_t *)strtol(&tokens[1][0], NULL, 16);
	sprintf(tx_buffer, "Saltando run a %#010x \r\n", jumpAddress);
	USART_putString(tx_buffer);
	runAssembly(jumpAddress);
	USART_putString("Al salir de run \r\n");
}
void call(void){
	jumpAddress =(uint32_t *)strtol(&tokens[1][0], NULL, 16);
	sprintf(tx_buffer, "Saltando call a %#010x \r\n", jumpAddress);
	USART_putString(tx_buffer);
	runAssembly(jumpAddress);
	USART_putString("Al salir de call \r\n");
}






/* I2c IOMAP Y UNMAP ----------------------------*/
void ioMap(void){
	USART_putString("I/O Map! \r\n");
	uint32_t in = strtol(tokens[1], NULL, 16);
	uint32_t out = strtol(tokens[2], NULL, 16);
	I2C_Write(SLAVE_ADDR,0x00,0X00);
	I2C_Write(SLAVE_ADDR,0x01,0xFF);
}
void ioUnmap(void){
	USART_putString("I/O Unmap! \r\n");
}







/* DISPLAY 7 SEGMENTOS */
void segmentOut(void){
	USART_putString("Segment Out! \r\n");
}




/* PANTALLA LCD*/
void lcdPrint(void){
	USART_putString("LCD Print! \r\n");
	//char * putString = " ";
	if(tokens[1]==NULL) tokens[1] = " ";
	if(tokens[2]==NULL) tokens[2] = " ";
	if(tokens[3]==NULL) tokens[3] = " ";
	if(tokens[4]==NULL) tokens[4] = " ";
	if(tokens[5]==NULL) tokens[5] = " ";
	if(tokens[6]==NULL) tokens[6] = " ";
	if(tokens[7]==NULL) tokens[7] = " ";
	if(tokens[8]==NULL) tokens[8] = " ";
	if(tokens[9]==NULL) tokens[9] = " ";
	if(tokens[10]==NULL) tokens[10] = " ";
	if(tokens[11]==NULL) tokens[11] = " ";
	if(tokens[12]==NULL) tokens[12] = " ";
	sprintf(lcd_buffer, "%s %s %s %s %s %s %s %s %s %s %s %s",
	tokens[1],tokens[2],tokens[3],tokens[4],tokens[5],tokens[6],
	tokens[7],tokens[8],tokens[9],tokens[10],tokens[11],tokens[12]);
	USART_putString(lcd_buffer);
	USART_putString("\r\n");
	sprintf(tx_buffer, "%.70s", lcd_buffer);
	/*if(tokens[1]!=0x00000000 && tokens[2]!=0x00000000 && tokens[3]!=0x00000000 &&  tokens[4]!=0x00000000){
		sprintf(putString, "%s %s %s %s %s", tokens[1],tokens[2],tokens[3],tokens[4]);
		Nokia5110_Clear();
		Nokia5110_SetCursor(0,1);
		Nokia5110_OutString(putString);
	}*/
	Nokia5110_Clear();
	Nokia5110_SetCursor(0,0);
	Nokia5110_OutString((unsigned char *)tx_buffer);
}




/* ---- BUZZER ---*/
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
/*funciones ayuda para buzzer*/
void PWM_SetFrequency(float freq_hz){
	dc = (2000000/freq_hz)-1;
	TIM16->ARR = (4000000/freq_hz)-1;
	TIM16->CCR1 = dc;
}

void PWM_SetDutyCycle(float dc_ms){
	dc = (SystemCoreClock/64)/((uint32_t)(1000/dc_ms));
	TIM16->CCR1 = dc;
}





//para HardFaultException
void Fault(void){
	USART_putString("Hard Fault, restart! \r\n");
}
