#ifndef _AD_9833_H
#define _AD_9833_H
#include "stdint.h"
//-------------------------------------------------------------------
// Константи для налаштування AD9833
#define AD9833_CMD_RESET    0x2100 // Скидання
#define AD9833_CMD_SINE     0x2000 // Синусоїдальний сигнал
#define AD9833_CMD_TRIANGLE 0x2002 // Трикутний сигнал
#define AD9833_CMD_SQUARE		0x2028 // Меандр сигнал
//-------------------------------------------------------------------
void AD9833_Init(void);
void AD9833_SetFrequency(uint32_t value, uint16_t value1);
void AD9833_SetType(uint16_t value);
void MCP1410_SetVolume(uint8_t value);
void MCP1410_ShowVolume(uint8_t value);
void AD9833_ShowType(uint16_t value);
void AD9833_MakeFreqString(uint32_t value, uint8_t *buf);
//------------------------------------------------
#endif
