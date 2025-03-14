#ifndef GLOBAL_LIB
#define GLOBAL_LIB
#include "stdint.h"
#include "stdio.h"
#include "AD9833/AD9833.h"
#include "USART\myusart.h" 
//********************************************************************************************
//********************************************************************************************
#define joy_no				0x00			//1900...2300			
#define joy_left			0x01			//0...500			
#define joy_right			0x02			//3600...4096			
#define joy_up				0x04			//3600...4096			
#define joy_down			0x08			//0...500
#define joy_error			0xFF			//preess time more then 5sec
//********************************************************************************************
#define enc_static		0xFFFF		//������� �� ������� ��� ��������
//********************************************************************************************
volatile static uint8_t	 	v_joy_state = joy_no;
volatile static uint8_t 	v_enc_percent;
//********************************************************************************************
//�������� ������� ������� � ������ ����������
volatile static uint8_t 	v_MCP1410_volume;
//********************************************************************************************
volatile static uint32_t 	v_AD9833_freq;
volatile static uint16_t 	v_AD9833_type;
//********************************************************************************************
//void IntToChar(uint8_t *buf, uint16_t num, uint8_t len);
//uint16_t CharHexToDec(char *str);
//********************************************************************************************
//����������� ��������� ��� ���
#define 	V_ref_cal		*(uint16_t*)0x1FFFF7BA
//********************************************************************************************
#endif
