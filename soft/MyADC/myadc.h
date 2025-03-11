#ifndef _My_ADC_H
#define _My_ADC_H
#include "stdint.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void my_ADC_Init(void);
void my_ADC2_Vref_Init(void);
uint8_t my_ADC_GetJoystick();
uint16_t my_ADC2_Vref_Get();
void my_ADC_MakeVolumeString(uint8_t *buf);
//-------------------------------------------------------------------
#endif
