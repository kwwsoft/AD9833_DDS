#include "stm32f30x.h"
#include "..\my_global.h"
#include "myencoder.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void my_Encoder_Init(){
 	GPIO_InitTypeDef port;
	TIM_TimeBaseInitTypeDef  enctim3;
	//
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	port.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	port.GPIO_Speed = GPIO_Speed_2MHz;
	port.GPIO_Mode = GPIO_Mode_AF;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &port);
	//
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_2);
	//
	//кнопка енкодера на вхід
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	port.GPIO_Pin = GPIO_Pin_0;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_IN;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &port);
	//
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	//
	TIM_TimeBaseStructInit(&enctim3);
	enctim3.TIM_ClockDivision = 1;
	enctim3.TIM_CounterMode = TIM_CounterMode_Down | TIM_CounterMode_Up;
	//значення з запасом - щоб не було перельотів по границях
	//за рахуноч темних ЕМС
	enctim3.TIM_Period = 260;
	//таке гарно валить дребезг
	enctim3.TIM_Prescaler = 4;
	enctim3.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,  &enctim3);
	//
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	//
	TIM_Cmd(TIM3, ENABLE);
}
	
//-------------------------------------------------------------------
volatile static uint32_t 	enc_val1 = enc_static;
volatile static uint32_t	enc_val2 = enc_static;
//-------------------------------------------------------------------
void my_Encoder_Set(uint8_t value){
	//встановити значення таймера для екодера
	TIM_SetCounter(TIM3, value);
}
//-------------------------------------------------------------------
uint32_t my_Encoder_Get(void){
	//беремо поточне значення таймера
	enc_val1 = TIM_GetCounter(TIM3);
	//був в нулі і перелетів на максимум
	if ( (enc_val2 == 0) && (enc_val1 > 100) ){
		//ставлю в 0
		TIM_SetCounter(TIM3, 0);
		return enc_static;
	}
	//якщо вийшло за межі 255
	if (enc_val1 > 255){
		//скидаємо таймер в 255 і вертаємо стан що енкодер без змін
		TIM_SetCounter(TIM3, 255);
		enc_val1 = 255;
		enc_val2 = 255;
		return enc_static;
	}
	//якщо не було змін
	if (enc_val1 == enc_val2)
		return enc_static;
	//були зміни в допустимих межах
	enc_val1 = TIM_GetCounter(TIM3);
	//збережу останній стан таймера
	enc_val2 = enc_val1;
	return enc_val1;
}
//-------------------------------------------------------------------
