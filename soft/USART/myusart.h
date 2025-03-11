#ifndef USART1_H_
#define USART1_H_
#include "stm32f30x.h"                  // Device header
//-------------------------------------------------------------------
void my_USART_Init(void);
void my_USART_Send(const uint8_t *pb);
//-------------------------------------------------------------------

#endif 
