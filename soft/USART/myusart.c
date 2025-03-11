#include "stm32f30x.h"                  // Device header
#include "myusart.h"
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
void my_USART_Init(void){
	USART_InitTypeDef usart;
 	GPIO_InitTypeDef port;
	//
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	//
	port.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_AF;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &port);
	//
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
//
	usart.USART_BaudRate = 115200;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &usart);
	//
	USART_Cmd(USART1, ENABLE);
}
//********************************************************************************************
void my_USART_Send(const uint8_t *pb){
	while(*pb){
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){
		}
		USART_SendData(USART1, *pb++);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){
		}
	}
}
//********************************************************************************************
