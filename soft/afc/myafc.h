#ifndef AFC_H_
#define AFC_H_
#include "stm32f30x.h"                  // Device header
#include "string.h"
#include ".\my_global.h"
#include ".\MyADC/myadc.h"
#include ".\AD9833/AD9833.h"
#include ".\HD44780/hd44780.h"
#include "TIM4/my_tim4_delay.h"
//-------------------------------------------------------------------
void my_AFC_Run(void);
void my_AFC_ProcessData(uint8_t *data, uint16_t len);
//-------------------------------------------------------------------

#endif 
