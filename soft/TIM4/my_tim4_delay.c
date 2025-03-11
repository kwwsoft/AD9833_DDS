#include "my_tim4_delay.h"
//***********************************************
volatile static uint8_t f_timer_4_end;
void TIM4_IRQHandler(void);
//***********************************************
void delay_TIM4_init(void){
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIMER_InitStructure.TIM_Prescaler = 8;
	TIMER_InitStructure.TIM_Period = 1;
	TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
	//3-1
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//***********************************************
void TIM4_IRQHandler(void)
{
 // extern volatile uint8_t f_timer_2_end;

  TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
  TIM4->SR &= ~TIM_SR_UIF;
  f_timer_4_end = 1;

  TIM_Cmd(TIM4, DISABLE);
  TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
}

//***********************************************
void delay_TIM4_ms(uint32_t n_msec)
{
  f_timer_4_end = 0;

  TIM4->PSC = 7200 - 1;
  TIM4->ARR = (uint16_t)(10 * n_msec);
  // для того чтобы установился PSC
  TIM4->EGR |= TIM_EGR_UG;
  TIM4->SR &= ~TIM_SR_UIF;

  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM4, ENABLE);

  while(f_timer_4_end == 0);
}
//***********************************************
void delay_TIM4_us(uint16_t n_usec)
{
  f_timer_4_end = 0;

  TIM4->PSC = 72;
  TIM4->ARR = n_usec-4;
  //TIM_Cmd(TIM4, ENABLE);

  // для того чтобы установился PSC
  TIM4->EGR |= TIM_EGR_UG;
  TIM4->SR &= ~TIM_SR_UIF;

  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM4, ENABLE);

  while(f_timer_4_end == 0);
}
//***********************************************

