#ifndef TIM4DELAY_LIB
#define TIM4DELAY_LIB

#include "stm32f30x.h"




void delay_TIM4_init(void);
void delay_TIM4_us(uint16_t n_usec);
void delay_TIM4_ms(uint32_t n_msec);

#endif
