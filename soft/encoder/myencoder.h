#ifndef _My_encoder_H
#define _My_encoder_H
#include "stdint.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void my_Encoder_Init(void);
uint32_t my_Encoder_Get(void);
void my_Encoder_Set(uint8_t value);
//-------------------------------------------------------------------
#endif
