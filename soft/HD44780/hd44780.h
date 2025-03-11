#ifndef HD444780_H_
#define HD444780_H_
#include "stm32f30x.h"                  // Device header
#include "stm32f30x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
//-------------------------------------------------------------------
#define hd44780_cursor_none		0x00
#define hd44780_cursor_place	0x01
#define hd44780_cursor_dash		0x02
//-------------------------------------------------------------------
void HD44780_Init(void);
void HD44780_Set_Pos(uint8_t line, uint8_t pos);
void HD44780_Out_String(uint8_t line, uint8_t pos, uint8_t *str);
void HD44780_Out_Char_At_Pos(uint8_t line, uint8_t pos, char str);
void HD44780_Out_Char(char str);
void HD44780_Clear(void);
void HD44780_Set_Cursor(uint8_t mode);
//void HD44780_Set_Entry_Mode(uint8_t cursor, uint8_t display);
//void HD44780_WriteMyCodePage();
//-------------------------------------------------------------------


#endif /* HD444780_H_ */
