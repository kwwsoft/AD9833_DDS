#ifndef _My_SPI_H
#define _My_SPI_H
#include "stdint.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void my_SPI_Init(void);
void my_SPI_AD9833(uint16_t data);
void my_SPI_MCP410(uint16_t data);
//-------------------------------------------------------------------
#endif
