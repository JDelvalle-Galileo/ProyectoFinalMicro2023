#include "stm32f3xx.h"                // Device header

#ifndef NOKIA5110_H_
#define NOKIA5110_H_

void Nokia5110_Init(void);
void Nokia5110_OutChar(unsigned char data);
void Nokia5110_OutString(unsigned char *ptr);
void Nokia5110_SetCursor(unsigned char newX, unsigned char newY);
void Nokia5110_Clear(void);
void Nokia5110_DrawFullImage(const char *ptr);


#endif /* NOKIA5110_H_ */
