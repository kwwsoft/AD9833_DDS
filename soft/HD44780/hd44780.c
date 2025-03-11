#include "stm32f30x.h" 	// Device header
#include "hd44780.h"
#include "..\TIM4\my_tim4_delay.h" 
//-------------------------------------------------------------------
#define HD44780_RS_Pin 					GPIO_Pin_10
#define HD44780_RS_GPIO_Port 		GPIOB
#define HD44780_RS_GPIO_RCC		  RCC_AHBPeriph_GPIOB
//-----
#define HD44780_E_Pin 					GPIO_Pin_11
#define HD44780_E_GPIO_Port 		GPIOB
#define HD44780_E_GPIO_RCC	 		RCC_AHBPeriph_GPIOB
//-----
#define HD44780_D4_Pin 					GPIO_Pin_12
#define HD44780_D5_Pin 					GPIO_Pin_13
#define HD44780_D6_Pin 					GPIO_Pin_14
#define HD44780_D7_Pin 					GPIO_Pin_15
#define HD44780_Data_GPIO_Port	GPIOB
#define HD44780_Data_GPIO_RCC	  RCC_AHBPeriph_GPIOB
//-------------------------------------------------------------------
#define hd44780_RS_L 	(HD44780_RS_GPIO_Port->BRR = HD44780_RS_Pin)  
#define hd44780_RS_H 	(HD44780_RS_GPIO_Port->BSRR = HD44780_RS_Pin) 
//-----
#define hd44780_E_L 	(HD44780_E_GPIO_Port->BRR = HD44780_E_Pin)  
#define hd44780_E_H 	(HD44780_E_GPIO_Port->BSRR = HD44780_E_Pin) 
//-----
#define hd44780_D4_L 	(HD44780_Data_GPIO_Port->BRR = HD44780_D4_Pin)  
#define hd44780_D4_H 	(HD44780_Data_GPIO_Port->BSRR = HD44780_D4_Pin) 
#define hd44780_D5_L 	(HD44780_Data_GPIO_Port->BRR = HD44780_D5_Pin)  
#define hd44780_D5_H 	(HD44780_Data_GPIO_Port->BSRR = HD44780_D5_Pin) 
#define hd44780_D6_L 	(HD44780_Data_GPIO_Port->BRR = HD44780_D6_Pin)  
#define hd44780_D6_H 	(HD44780_Data_GPIO_Port->BSRR = HD44780_D6_Pin) 
#define hd44780_D7_L 	(HD44780_Data_GPIO_Port->BRR = HD44780_D7_Pin)  
#define hd44780_D7_H 	(HD44780_Data_GPIO_Port->BSRR = HD44780_D7_Pin) 
//-------------------------------------------------------------------
void HD44780_SendByte(uint8_t data);
void HD44780_SendCommand(uint8_t data);
void HD44780_SendData(uint8_t data);
void HD44780_WriteMyCodePage(void);
//-------------------------------------------------------------------
//cursor mode
//0x0000 1DBC
//D=1 зображення є
//C=1 курсор прочерк ввімкнено
//B=1 курсор моргаюче знакомісце ввімкнено
volatile uint8_t HD44780_Cursor = 0x0C;
//-------------------------------------------------------------------
// 5x8
unsigned char __cgram[]=
{
0x0a, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, //Char0 'Ї'
0x0e, 0x11, 0x11, 0x11, 0x0a, 0x0a, 0x1b, 0x00, //Char1 omega
0x00, 0x04, 0x0d, 0x0b, 0x0a, 0x08, 0x10, 0x00, //Char2 мю мікро
0x15, 0x15, 0x15, 0x0E, 0x15, 0x15, 0x15, 0x00, //Char3 'Ж'
0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00, //Char4 'И'
0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00, //Char5 'Л'
0x00, 0x04, 0x0A, 0x11, 0x11, 0x00, 0x00, 0x00, //Char7 sinus up
0x00, 0x00, 0x00, 0x01, 0x11, 0x0A, 0x0E, 0x00, //Char6 sinus down
};
//-------------------------------------------------------------------
void HD44780_SendByte(uint8_t data){
	if (data & 0x80){
		hd44780_D7_H;
	}else{
		hd44780_D7_L;
	}
	if (data & 0x40){
		hd44780_D6_H;
	}else{
		hd44780_D6_L;
	}
	if (data & 0x20){
		hd44780_D5_H;
	}else{
		hd44780_D5_L;
	}
	if (data & 0x10){
		hd44780_D4_H;
	}else{
		hd44780_D4_L;
	}
	hd44780_E_H;
	delay_TIM4_us(400);
	hd44780_E_L;
	delay_TIM4_us(400);
	//
	if (data & 0x08){
		hd44780_D7_H;
	}else{
		hd44780_D7_L;
	}
	if (data & 0x04){
		hd44780_D6_H;
	}else{
		hd44780_D6_L;
	}
	if (data & 0x02){
		hd44780_D5_H;
	}else{
		hd44780_D5_L;
	}
	if (data & 0x01){
		hd44780_D4_H;
	}else{
		hd44780_D4_L;
	}
	hd44780_E_H;
	delay_TIM4_us(400);
	hd44780_E_L;
	delay_TIM4_us(400);
}
//-----------------------------------------------------------------------------
void HD44780_SendCommand(uint8_t data){
	hd44780_RS_L;
	delay_TIM4_ms(1);
	HD44780_SendByte(data);
	delay_TIM4_ms(1);
}
//-----------------------------------------------------------------------------
void HD44780_SendData(uint8_t data){
	hd44780_RS_H;
	delay_TIM4_ms(1);
	HD44780_SendByte(data);
	delay_TIM4_ms(1);
}
//-----------------------------------------------------------------------------
void HD44780_Init(void){
	//init HD44780 data port
	GPIO_InitTypeDef port;
	GPIO_StructInit(&port);
	RCC_AHBPeriphClockCmd(HD44780_Data_GPIO_RCC, ENABLE);
	port.GPIO_Pin = HD44780_D4_Pin | HD44780_D5_Pin | HD44780_D6_Pin | HD44780_D7_Pin;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(HD44780_Data_GPIO_Port, &port);
	//init HD44780 RS port
	GPIO_StructInit(&port);
	RCC_APB2PeriphClockCmd(HD44780_RS_GPIO_RCC, ENABLE);
	port.GPIO_Pin = HD44780_RS_Pin;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(HD44780_RS_GPIO_Port, &port);
	//init HD44780 E port
	GPIO_StructInit(&port);
	RCC_APB2PeriphClockCmd(HD44780_E_GPIO_RCC, ENABLE);
	port.GPIO_Pin = HD44780_E_Pin;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(HD44780_E_GPIO_Port, &port);
	//
	GPIO_ResetBits(HD44780_Data_GPIO_Port, HD44780_D4_Pin|HD44780_D5_Pin|HD44780_D6_Pin|HD44780_D7_Pin);
	hd44780_E_L; 
	hd44780_RS_L;	
	delay_TIM4_ms(20);
	HD44780_SendCommand(0x03);
	delay_TIM4_ms(20);
	HD44780_SendCommand(0x03);
	delay_TIM4_ms(10);
	HD44780_SendCommand(0x03);
	delay_TIM4_ms(10);
	HD44780_SendCommand(0x02);
	delay_TIM4_ms(10);
	HD44780_SendCommand(0x28);
	delay_TIM4_ms(10);
	HD44780_SendCommand(HD44780_Cursor);//cursor off
	delay_TIM4_ms(10);
	HD44780_SendCommand(0x01);//display clear
	delay_TIM4_ms(10);
	HD44780_SendCommand(0x06);//entry mode set
	delay_TIM4_ms(10);
	//
	HD44780_Clear();
	//HD44780_WriteMyCodePage();
}
//-------------------------------------------------------------------
void HD44780_Clear(void){
 HD44780_SendCommand(0x01);//display clear
}
//-----------------------------------------------------------------------------
void HD44780_Out_String(uint8_t line, uint8_t pos, uint8_t *str){
// line 0..1    pos 0...15
	if (line == 0){
		HD44780_SendCommand(0x80);
	}
	else{
		HD44780_SendCommand(0xC0);
	}
	while(pos-- > 0){
		HD44780_SendCommand(0x14);
	}
	while(*str){
		HD44780_SendData(*str++);
	}
}
//-----------------------------------------------------------------------------
void HD44780_Out_Char_At_Pos(uint8_t line, uint8_t pos, char str){
// line 0..1    pos 0...15
	if (line == 0){
		HD44780_SendCommand(0x80);
	}
	else{
		HD44780_SendCommand(0xC0);
	}
	while(pos-- > 0){
		HD44780_SendCommand(0x14);
	}
	HD44780_SendData(str);
}
//-----------------------------------------------------------------------------
void HD44780_Out_Char(char str){
	HD44780_SendData(str);
}
//-----------------------------------------------------------------------------
void HD44780_Set_Pos(uint8_t line, uint8_t pos){
// line 0..1    pos 0...15
	if (line == 0){
		HD44780_SendCommand(0x80);
	}
	else{
		HD44780_SendCommand(0xC0);
	}
	while(pos-- > 0){
		HD44780_SendCommand(0x14);
	}
}
//-----------------------------------------------------------------------------
void HD44780_Set_Entry_Mode(uint8_t cursor, uint8_t display){
	//default
	//HD44780_SendCommand(0x06);//entry mode set  //10
	//0000 01[ID][S]
	//[ID] - cursor  1 - курсор вправо, 0 - курсов вліво
	//[S] - display  1 - рухається дисплей а не курсор, 0 - рухається курсор
	HD44780_SendCommand((uint8_t)(0x04 | (cursor<<1) | display));//entry mode set
}
//-----------------------------------------------------------------------------
void HD44780_WriteMyCodePage(){
//перші 8 знакомість пишемо свої символи 5х8	
	//писати в память з 0 мій знакогенератор
	HD44780_SendCommand(0x40);
	delay_TIM4_us(100);
	//8 символів - загально 64 байт
	for (uint8_t i = 0; i < 64; i++){
		HD44780_SendData(__cgram[i]);
	  delay_TIM4_us(100);
	}
  delay_TIM4_us(100);
	//переставити вказівник на початок занкогенератора - початок всієї таблиці
	HD44780_SendCommand(0xC0);
	//мої символи в таблиці з 0 по 7. HD44780_Out_Char(3) - вивести третій
}
//-----------------------------------------------------------------------------
void HD44780_Set_Cursor(uint8_t mode){
//mode=0 немає, mode=2 знакомісце, mode=1 прочерк	
	switch(mode){
		//mode=1 
		case 1:
				HD44780_Cursor |= 0x01;
				HD44780_Cursor &= ~0x02;
			break;
		//
		case 2:
				HD44780_Cursor &= ~0x01;
				HD44780_Cursor |= 0x02;
			break;
		//немає
		default:
				HD44780_Cursor &= ~0x03;
			break;
	}
	HD44780_SendCommand(HD44780_Cursor);
}
//-----------------------------------------------------------------------------



