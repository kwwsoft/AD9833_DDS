#ifndef _My_ADC_H
#define _My_ADC_H
#include "stdint.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void my_ADC_Init(void);
void my_ADC_Start();
uint32_t my_Read_Vdda_mV(void);
uint8_t my_ADC_GetJoystick();
void my_ADC_MakeVolumeString(uint8_t *buf);
uint16_t GetAttValue();
//-------------------------------------------------------------------
#endif
