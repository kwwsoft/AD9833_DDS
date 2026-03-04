#ifndef USART1_H_
#define USART1_H_
#include "stm32f30x.h"                  // Device header
//-------------------------------------------------------------------
void my_USART1_Init(void);
void my_USART1_DMA_RX_Init(void);
void my_USART1_DMA_TX_Init(/*uint8_t *buffer, uint16_t size*/);
void my_USART1_DMA_Send(uint8_t *buf, uint8_t len);
void my_USART1_Send(uint8_t *data, uint16_t len);
//-------------------------------------------------------------------

#endif 
