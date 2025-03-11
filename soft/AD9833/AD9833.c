#include "stm32f30x.h"                  // Device header
#include "stdio.h"
#include "string.h"
#include "my_global.h"
#include "..\MySPI\myspi.h"
#include "HD44780/hd44780.h"
#include "AD9833.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
// ������������ ������� �� AD9833
void AD9833_SetFrequency(uint32_t value, uint16_t value1) {
    // ���������� ������� ������� (25 ��� ������ �������)
	  uint32_t freqReg =  (uint32_t)((float)((value * 268435456.0) / 25000000.0)); 
    uint16_t freqLSB = (int)(freqReg & 0x3FFF);       // ������� 14 ��
    uint16_t freqMSB = (int)((freqReg & 0xFFFC000) >> 14); // ������ 14 ��
		my_SPI_AD9833(0x2100);                // ���������� �������
    my_SPI_AD9833(0x4000 | freqLSB);      // ������� ���
    my_SPI_AD9833(0x4000 | freqMSB);      // ������ ���
    my_SPI_AD9833(value1);       // �������� ������������� ������
}
//-------------------------------------------------------------------
void AD9833_Init(){
  // �������� AD9833
  my_SPI_AD9833(AD9833_CMD_RESET);
}
//-------------------------------------------------------------------
void AD9833_SetType(uint16_t value){
	switch(value){
		case AD9833_CMD_RESET:
		case AD9833_CMD_SINE:
		case AD9833_CMD_SQUARE:
		case AD9833_CMD_TRIANGLE:
			my_SPI_AD9833(value);
		break;
		//hz
		default:
		break;
	}
}
//-------------------------------------------------------------------
void MCP1410_SetVolume(uint8_t value){
	//0...255 �������� ����������
	uint16_t volume = 0x1100;
	volume |= value;
	my_SPI_MCP410(volume);
}
//------------------------------------------------------------------- 
void MCP1410_ShowVolume(uint8_t value){
	uint8_t buf[5];
	//������� �������� �������� � �������� � �����
/*	IntToChar(buf, value, 3);
	buf[3] = '%';
	buf[4] = 0;
	//�������� ������� �������� ��� ���� ���� �
	if (buf[0] == '0'){
		buf[0] = ' ';
		if (buf[1] == 0){
			buf[1] = ' ';
		}
	}*/
	sprintf((char*)buf, "%3d%c", value, '%');
	//������ � ������ ������� �� �������	 
	HD44780_Out_String(1, 12, buf);
}
//------------------------------------------------------------------- 
void AD9833_ShowType(uint16_t value){
	//������� �� ���� ������� ������� �� ���� ������
	if (value == AD9833_CMD_SINE){
		HD44780_Out_String(0, 13, (uint8_t*)"SIN");
	}
	else if (value == AD9833_CMD_TRIANGLE){
		HD44780_Out_String(0, 13, (uint8_t*)"TRI");
	}
	else if (value == AD9833_CMD_SQUARE){
		HD44780_Out_String(0, 13, (uint8_t*)"SQR");
	}
}
//-------------------------------------------------------------------
void AD9833_MakeFreqString(uint32_t value, uint8_t *buf){
	//���������� ������ ������� � ����� �������� 11 ����
	//������� ���� [10] - ����
	//������� ���� ����� 465000 � ������ "00.465.000"
	char b1[16];
	uint8_t len, pos;
	//����������� �� ���� 00.000.000
	for (len = 0; len < 10; len++)
		buf[len] = '0';
	buf[2] = '.';
	buf[6] = '.';
	//������� � �������� ����� � ������
	//������� ������� ������������ ����
	sprintf(b1, "%d", value);
	//������� ������� ������������ ����
	len = strlen(b1);
	if (len == 0)
		return;
	//
	pos = 9;
	//����������� � �������� ����� � ����
	do{
		if(pos == 6)
			pos --;
		if (pos == 2)
			pos --;
		//
		buf[pos--] = b1[--len];
	}while(len > 0); 
}
//-------------------------------------------------------------------




