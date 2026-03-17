#include "stm32f30x.h"                  // Device header
#include "./afc/myafc.h"
#include "myusart.h"
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
void USART1_ProcessData(uint8_t *data, uint16_t len);
//********************************************************************************************
//buffer size for receiving via DMA
#define RX_BUF_SIZE 256
uint8_t usart1_rx_dma_buf[RX_BUF_SIZE];
volatile uint16_t usart1_rx_old_pos = 0;
//********************************************************************************************
void my_USART1_Init(void){
	USART_InitTypeDef usart;
 	GPIO_InitTypeDef port;
  NVIC_InitTypeDef  NVIC_InitStruct;
	//
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	//UART_Tx  USART_Rx
	port.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_AF;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &port);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);
	//
  usart.USART_BaudRate = 460800;
  //usart.USART_BaudRate = 115200;
  usart.USART_WordLength = USART_WordLength_8b;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_Parity = USART_Parity_No;
  usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART1, &usart);
	//allow bus idle interrupts
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	//reception and transmission is via DMA
  USART_DMACmd(USART1, USART_DMAReq_Rx/* | USART_DMAReq_Tx*/, ENABLE);
	//allow interrupts from usart
  NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
	//
  USART_Cmd(USART1, ENABLE);
}
//********************************************************************************************
void my_USART1_DMA_RX_Init(void){
    DMA_InitTypeDef DMA_InitStruct;
		//clock the DMA bus
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel5);
		//reception via DMA into buffer
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->RDR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)usart1_rx_dma_buf;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = RX_BUF_SIZE;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
		//enable DMA channel for usart
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);
    DMA_Cmd(DMA1_Channel5, ENABLE);
}
//********************************************************************************************
void my_USART1_DMA_TX_Init(/*uint8_t *buffer, uint16_t size*/){
    DMA_InitTypeDef DMA_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel4);
		//configure DMA transmission via a pre-allocated buffer
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->TDR;
    //DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buffer;
    DMA_InitStruct.DMA_MemoryBaseAddr = 0;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    //DMA_InitStruct.DMA_BufferSize = size;
    DMA_InitStruct.DMA_BufferSize = 0;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
		//allow DMA transmission
    DMA_Init(DMA1_Channel4, &DMA_InitStruct);
}
//********************************************************************************************
void my_USART1_DMA_Send(uint8_t *buf, uint8_t len){
	
	while(DMA_GetCurrDataCounter(DMA1_Channel4) != 0){};
	
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = len;
	DMA1_Channel4->CMAR = (uint32_t)buf;
	
   DMA_ClearFlag(DMA1_FLAG_TC4);
  
    DMA_Cmd(DMA1_Channel4, ENABLE);



/*
while(DMA_GetCurrDataCounter(DMA1_Channel4) != 0){};
	//packet size and start of DMA transmission
  DMA_SetCurrDataCounter(DMA1_Channel4, size);
  DMA_Cmd(DMA1_Channel4, ENABLE);
	*/
}
//********************************************************************************************
void USART1_IRQHandler(void){
//when the bus is idle - there is no transmission or reception - then this interrupt is triggered
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET){
        volatile uint32_t tmp;
        uint16_t pos, len;
				//dumb read to reset interrupt
        //tmp = USART1->ISR;
        //tmp = USART1->RDR;
        //(void)tmp;
				USART_ClearITPendingBit(USART1, USART_IT_IDLE);
        pos = RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
        if (pos != usart1_rx_old_pos)
        {
            if (pos > usart1_rx_old_pos)
            {
                len = pos - usart1_rx_old_pos;
                my_AFC_ProcessData(&usart1_rx_dma_buf[usart1_rx_old_pos], len);
            }
            else
            {
                len = RX_BUF_SIZE - usart1_rx_old_pos;
                my_AFC_ProcessData(&usart1_rx_dma_buf[usart1_rx_old_pos], len);

                if (pos > 0)
                {
                    my_AFC_ProcessData(&usart1_rx_dma_buf[0], pos);
                }
            }

            usart1_rx_old_pos = pos;
        }
    }
}
//********************************************************************************************
/*void USART1_ProcessData(uint8_t *data, uint16_t len)
{
    // Example: echo back
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = data[i];
    }
}*/
//********************************************************************************************
void my_USART1_Send(uint8_t *data, uint16_t len){
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = data[i];
    }
}
//********************************************************************************************
