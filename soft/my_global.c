#include "stm32f30x.h"                  // Device header
#include "string.h"
#include "my_global.h"
//***********************************************

//extern volatile uint32_t v_systick;
//***********************************************
/*uint16_t CharHexToDec(char *str){
	uint16_t num = 0;
	//x00
	if (str[0] > 0x40){
		num = (str[0] - 0x37) * 256;
	}
	else{
		num = (str[0] - 0x30) * 256;
	}
	//0x0
	if (str[1] > 0x40){
		num += (str[1] - 0x37) * 16;
	}
	else{
		num += (str[1] - 0x30) * 16;
	}
	//00x
	if (str[2] > 0x40){
		num += (str[2] - 0x37);
	}
	else{
		num += (str[2] - 0x30);
	}
	return num;
}*/
//***********************************************
/*void IntToChar(uint8_t *buf, uint16_t num, uint8_t len){

	have int 0...999 and convert to char array
	refrence to buf for output
	lenght 1..3 in len
	
   int n1;
    buf[1] = ' ';
    buf[0] = ' ';
    if (len == 1) {
        buf[2] = num % 10 + 0x30;
    }
    else if (len == 2) {
        buf[2] = num % 10 + 0x30;
        n1 = num / 10;
        buf[1] = n1 % 10 + 0x30;
    }
    else{
        buf[2] = num % 10 + 0x30;
        n1 = num / 10;
        buf[1] = n1 % 10 + 0x30;
        buf[0] = n1 / 10 + 0x30;
    }
}*/
//***********************************************



