#include "stm32f30x.h"
//#include "..\my_global.h"
#include "myspi.h"
//-------------------------------------------------------------------
//CLK						GPIO_Pin_3
//MCP410_CS			GPIO_Pin_4
//DAT						GPIO_Pin_5
//AD9833_CS			GPIO_Pin_6
//-------------------------------------------------------------------
#define AD9833_CS_PIN     		GPIO_Pin_6  
#define AD9833_CS_PORT    		GPIOB

#define MCP410_CS_PIN     		GPIO_Pin_4  
#define MCP410_CS_PORT    		GPIOB

#define CLK_PIN 		    			GPIO_Pin_3  
#define CLK_PORT    					GPIOB
#define DAT_PIN 		    			GPIO_Pin_5  
#define DAT_PORT    					GPIOB
//-------------------------------------------------------------------
// ������� ��� ��������� CS AD9833
#define AD9833_CS_LOW()  (AD9833_CS_PORT->BRR = AD9833_CS_PIN)
#define AD9833_CS_HIGH() (AD9833_CS_PORT->BSRR = AD9833_CS_PIN)
// ������� ��� ��������� CS MCP410
#define MCP410_CS_LOW()  (MCP410_CS_PORT->BRR = MCP410_CS_PIN)
#define MCP410_CS_HIGH() (MCP410_CS_PORT->BSRR = MCP410_CS_PIN)
//-------------------------------------------------------------------
// ������� ��� ��������� CLK
#define spi_CLK_LOW()  (CLK_PORT->BRR = CLK_PIN)
#define spi_CLK_HIGH() (CLK_PORT->BSRR = CLK_PIN)
// ������� ��� ��������� DAT
#define spi_DAT_LOW()  (DAT_PORT->BRR = DAT_PIN)
#define spi_DAT_HIGH() (DAT_PORT->BSRR = DAT_PIN)
//-------------------------------------------------------------------
#define MCP410_Delay()  __asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
//-------------------------------------------------------------------
void my_SPI_Init(){
 	GPIO_InitTypeDef port;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	port.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_6;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &port);
	//������ ���������� - ���� �� ������
	AD9833_CS_HIGH();
	MCP410_CS_HIGH();
}
	
//-------------------------------------------------------------------
void my_SPI_AD9833(uint16_t data){
	uint16_t byte = 0x8000;
	//������ �� ������� �����
	AD9833_CS_HIGH();
	spi_CLK_HIGH();
	spi_DAT_HIGH();
	//������� �������� - ������� ���
	AD9833_CS_LOW();
	//��������� �� 16 �� ������� ������
	do{
		//0
		if ( (data & byte) == 0){
			spi_DAT_LOW();
		}
		//1
		else{
			spi_DAT_HIGH();
		}
		//�������� ��������� ���� ���� ���������
		//������� ����� �� ����� �����
		spi_CLK_LOW();
		//������ ������� ���� �� ����� 80�� - ��� �� ����������
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
		//������ ����
		spi_CLK_HIGH();
		//������� �� ��������� ���. 1 ����� ���� �������
		byte >>= 1;
	//���� �� ������� �� ���	
	}while( byte != 0);
	//�������� ��� ������� ���������� �� ����� 80�� - ��� �� ����������
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//��������� ���
	AD9833_CS_HIGH();
	spi_CLK_HIGH();
	spi_DAT_HIGH();
}
//-------------------------------------------------------------------
void my_SPI_MCP410(uint16_t data){
	uint16_t byte = 0x8000;
	//��������� �� ������� �����
	MCP410_CS_HIGH();
	spi_CLK_LOW();
	spi_DAT_HIGH();

	//������� �������� - ������� ���
	MCP410_CS_LOW();
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//��������� �� 16 �� ������� ������
	do{
		//0
		if ( (data & byte) == 0){
			spi_DAT_LOW();
		}
		//1
		else{
			spi_DAT_HIGH();
		}
		//�������� ��������� ���� ���� ���������
		//������� ����� �� ������ �����
		spi_CLK_HIGH();
		//������ ������� ���� �� ����� 130�� - ��� �� ����������
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
		//������ ������� ����
		spi_CLK_LOW();
		//������� �� ��������� ���. 1 ����� ���� �������
		byte >>= 1;
	//���� �� ������� �� ���	
	}while( byte != 0);
	//�������� ��� ������� ���������� �� ����� 80�� - ��� �� ����������
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//��������� ���
	MCP410_CS_HIGH();
	spi_CLK_LOW();
	spi_DAT_HIGH();
}
//-------------------------------------------------------------------
