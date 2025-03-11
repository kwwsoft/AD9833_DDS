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
// Макроси для керування CS AD9833
#define AD9833_CS_LOW()  (AD9833_CS_PORT->BRR = AD9833_CS_PIN)
#define AD9833_CS_HIGH() (AD9833_CS_PORT->BSRR = AD9833_CS_PIN)
// Макроси для керування CS MCP410
#define MCP410_CS_LOW()  (MCP410_CS_PORT->BRR = MCP410_CS_PIN)
#define MCP410_CS_HIGH() (MCP410_CS_PORT->BSRR = MCP410_CS_PIN)
//-------------------------------------------------------------------
// Макроси для керування CLK
#define spi_CLK_LOW()  (CLK_PORT->BRR = CLK_PIN)
#define spi_CLK_HIGH() (CLK_PORT->BSRR = CLK_PIN)
// Макроси для керування DAT
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
	//підняти чіпселекти - чіпи не вибрані
	AD9833_CS_HIGH();
	MCP410_CS_HIGH();
}
	
//-------------------------------------------------------------------
void my_SPI_AD9833(uint16_t data){
	uint16_t byte = 0x8000;
	//підняти всі сигнали вверх
	AD9833_CS_HIGH();
	spi_CLK_HIGH();
	spi_DAT_HIGH();
	//початок передачі - вибрати чіп
	AD9833_CS_LOW();
	//послідовно всі 16 біт старший перший
	do{
		//0
		if ( (data & byte) == 0){
			spi_DAT_LOW();
		}
		//1
		else{
			spi_DAT_HIGH();
		}
		//затримки виконання коду вище достатньо
		//фіксуємо рівень по спаду клока
		spi_CLK_LOW();
		//тримаю низьким клок не менше 80нс - так по аналізатору
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
		//підняти клок
		spi_CLK_HIGH();
		//перехід до слідуючого біта. 1 біжить зліва направо
		byte >>= 1;
	//поки не вибіжать всі біти	
	}while( byte != 0);
	//затримка для підняття чіпселекта не менше 80нс - так по аналізатору
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//відключити чіп
	AD9833_CS_HIGH();
	spi_CLK_HIGH();
	spi_DAT_HIGH();
}
//-------------------------------------------------------------------
void my_SPI_MCP410(uint16_t data){
	uint16_t byte = 0x8000;
	//встановив всі сигнали вверх
	MCP410_CS_HIGH();
	spi_CLK_LOW();
	spi_DAT_HIGH();

	//початок передачі - вибрати чіп
	MCP410_CS_LOW();
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//послідовно всі 16 біт старший перший
	do{
		//0
		if ( (data & byte) == 0){
			spi_DAT_LOW();
		}
		//1
		else{
			spi_DAT_HIGH();
		}
		//затримки виконання коду вище достатньо
		//фіксуємо рівень по фронту клока
		spi_CLK_HIGH();
		//тримаю високим клок не менше 130нс - так по аналізатору
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
		//підняти скинути клок
		spi_CLK_LOW();
		//перехід до слідуючого біта. 1 біжить зліва направо
		byte >>= 1;
	//поки не вибіжать всі біти	
	}while( byte != 0);
	//затримка для підняття чіпселекта не менше 80нс - так по аналізатору
	__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");__asm volatile("NOP");
	//відключити чіп
	MCP410_CS_HIGH();
	spi_CLK_LOW();
	spi_DAT_HIGH();
}
//-------------------------------------------------------------------
