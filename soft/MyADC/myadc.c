#include "stm32f30x.h"
#include "stdio.h"
#include "string.h"
#include "my_global.h"
#include "USART\myusart.h" 
#include "..\TIM4\my_tim4_delay.h" 
#include "myadc.h"
//-------------------------------------------------------------------

#define PUTCHAR_PROTOTYPE1 int fputc(int ch, FILE *f)
PUTCHAR_PROTOTYPE1
{
	my_USART_Send((uint8_t*)(&ch));;
  return ch;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
extern volatile uint32_t sysTickCount;
static volatile uint32_t sysTickInterval;
//-------------------------------------------------------------------
uint16_t v_x, v_y;	
uint16_t v_att;	
uint16_t v_out_adc;	
//-------------------------------------------------------------------
void my_ADC_Init(){
 	GPIO_InitTypeDef port;
  ADC_InjectedInitTypeDef adc;
	//
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	port.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2 | GPIO_Pin_3;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_AN;
	port.GPIO_OType = GPIO_OType_OD;
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &port);
	//кнопка джойстика на вхід
	port.GPIO_Pin = GPIO_Pin_5;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	port.GPIO_Mode = GPIO_Mode_IN;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &port);

	//
  // Увімкнення тактування для ADC12
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	//reset voltage regulator
	ADC_VoltageRegulatorCmd(ADC1, DISABLE);
	ADC_VoltageRegulatorCmd(ADC2, DISABLE);
	delay_TIM4_us(100);
	ADC_VoltageRegulatorCmd(ADC1, ENABLE);
	ADC_VoltageRegulatorCmd(ADC2, ENABLE);
	delay_TIM4_us(100);
  //Calibration adc1_2
	ADC1->CR &= ~ADC_CR_ADCALDIF;
	ADC2->CR &= ~ADC_CR_ADCALDIF;
  ADC1->CR |= ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);
  ADC2->CR |= ADC_CR_ADCAL;
  while(ADC2->CR & ADC_CR_ADCAL);
	// Налаштування базових параметрів ADC
	ADC_InjectedStructInit(&adc);
	adc.ADC_ExternalTrigInjecConvEvent = ADC_ExternalTrigConvEvent_0;
	adc.ADC_ExternalTrigInjecEventEdge = ADC_ExternalTrigEventEdge_None;
	adc.ADC_InjecSequence1 = ADC_InjectedChannel_1;
	adc.ADC_InjecSequence2 = ADC_InjectedChannel_2;
	adc.ADC_InjecSequence3 = ADC_InjectedChannel_3;
	adc.ADC_InjecSequence4 = ADC_InjectedChannel_4;
	adc.ADC_NbrOfInjecChannel = 4;
	ADC_InjectedInit(ADC1, &adc);
	//
	//джойстик вісь х
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_1, ADC_SampleTime_7Cycles5);
	//джойстик вісь y
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_2, ADC_SampleTime_7Cycles5);
	//положення перемикача атенюатора
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_3, ADC_SampleTime_7Cycles5);
	//напруга на виході генератора з детектора
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_4, ADC_SampleTime_601Cycles5);
	// Увімкнення інжекторного режиму
	ADC_AutoInjectedConvCmd(ADC1, ENABLE);
	//
	// Увімкнення ADC
	ADC_Cmd(ADC1, ENABLE);
	// Очікування готовності ADC
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
}
	
//-------------------------------------------------------------------
void my_ADC2_Vref_Init(void){
  ADC_InitTypeDef adc;
	//
	adc.ADC_AutoInjMode = DISABLE;
	adc.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
	adc.ADC_DataAlign = ADC_DataAlign_Right;
	adc.ADC_ExternalTrigConvEvent = ADC_ExternalTrigInjecEventEdge_None;
	adc.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	adc.ADC_NbrOfRegChannel = 1;
	adc.ADC_OverrunMode = DISABLE;
	adc.ADC_Resolution = ADC_Resolution_12b;
	ADC_Init(ADC2, &adc);
	//опорна напруга АЦП
	ADC_VrefintCmd(ADC2, ENABLE);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_Vrefint, 1, ADC_SampleTime_601Cycles5);
	//
	ADC_Cmd(ADC2, ENABLE);
	// Очікування готовності ADC
	while (!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));
}
//-------------------------------------------------------------------
void my_ADC_Start(){
	ADC_StartInjectedConversion(ADC1);
  // Очікування завершення конверсії
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC));
	//get joystick value
	v_x = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
	v_y = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2);
	v_att = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_3);
	v_out_adc = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_4);
}
//-------------------------------------------------------------------
uint16_t my_ADC2_Vref_Get(){
	ADC_StartConversion(ADC2);
  // Очікування завершення конверсії
  while (!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC2);
}
//-------------------------------------------------------------------
uint8_t my_ADC_GetJoystick(){
	//
	my_ADC_Start();
	//якщо була помлк по таймауту
	if (v_joy_state == joy_error){
		//зажатий по координаті - помилка
		if ( (v_x < 1900) || (v_x > 2300) ){
			v_joy_state = joy_error;
			return joy_error;
		}
		//зажатий по координаті - помилка
		else if ( (v_y < 1900) || (v_y > 2300) ){
			v_joy_state = joy_error;
			return joy_error;
		}
		//бв в помилці - зараз по центру
		else{
			v_joy_state = joy_no;
			return joy_no;
		}
	}
	//нічого не нажато
	if ( (v_x > 1900) && (v_x < 2300) ){
		if ( (v_y > 1900) && (v_y < 2300) ){
			v_joy_state = joy_no;
			return joy_no;
		}
	}
	//є щось нажате - начинаю моніторинг
	//5sec
	sysTickInterval = sysTickCount + 6000;
	//left
	if (v_x < 500){
		//очікую 5сек на відпускання
		do{
			my_ADC_Start();
			//5сек пройшло - кнопка зажата. то є помилка
			if (sysTickInterval < sysTickCount){
				v_joy_state = joy_error;
				return joy_error;
			}
		}while(v_x < 1900);
		//відпустив - вернути 
		v_joy_state = joy_no;
		return joy_left;
	}
	//down
	else if (v_y < 500){
		//очікую на відпускання
		do{
			my_ADC_Start();
			//5сек пройшло - кнопка зажата. то є помилка
			if (sysTickInterval < sysTickCount){
				v_joy_state = joy_error;
				return joy_error;
			}
		}while(v_y < 1900);
		//відпустив - вернути 
		v_joy_state = joy_no;
		return joy_down;
	}
	//right
	else if (v_x > 3600){
		//очікую на відпускання
		do{
			my_ADC_Start();
			//5сек пройшло - кнопка зажата. то є помилка
			if (sysTickInterval < sysTickCount){
				v_joy_state = joy_error;
				return joy_error;
			}
		}while(v_x > 2300);
		//відпустив - вернути 
		v_joy_state = joy_no;
		return joy_right;
	}
	//up
	else if (v_y > 3600){
		//очікую на відпускання
		do{
			my_ADC_Start();
			//5сек пройшло - кнопка зажата. то є помилка
			if (sysTickInterval < sysTickCount){
				v_joy_state = joy_error;
				return joy_error;
			}
		}while(v_y > 2300);
		//відпустив - вернути 
		v_joy_state = joy_no;
		return joy_up;
	}
	return joy_no;;
}
//-------------------------------------------------------------------
uint16_t GetAttValue(){
//повернути дільник по напрузі 0,1,10,100,1000	
	//всі вимкнуті
	if (v_att > 3750){
		return 0;
	}
	//1:1000
	else if (v_att > 3000){
		return 1000;
	}
	//1:100
	else if (v_att > 2100){
		return 100;
	}
	//1:10
	else if (v_att > 1300){
		return 10;
	}
	//1:1
	return 1;
}
//-------------------------------------------------------------------
void my_ADC_MakeVolumeString(uint8_t *buf){
	//напруга - ціле число в мілівольтах
	//або через дільник в мікровольтах
	//
	char b1[16];
 	uint8_t len, pos;
	//напруга живлення в мілівольтах
	float v_Vdda;
	float v2;
	//напруга
	uint32_t v1;
	//взяти опорне
	v1 = my_ADC2_Vref_Get();
	//вирахувати живлення в мВ
	v_Vdda = (3300.0 * (float)V_ref_cal  / (float)v1);
	//напруга на виході детектора в мілівольтах
	v2 = v_Vdda * (float)v_out_adc / 4095.0;
//	printf("v_det: %d\r\n", v1);
	//вибираємо похибку по рівнянню прямої
	//по результатах калібровки
	v1 = (uint32_t)(v2 * 0.75011 + 36.722);
//	printf("v_calc: %d\r\n", v1);
	//00000
	//очиистити вихідний буфер
	for (len = 0; len < 5; len++)
		buf[len] = ' ';
	buf[5] = 0;
	//індикація в залежності від атенюатора
	switch(GetAttValue()){
		//1:1
		//вивід в мілівольтах
		case 1:
			//конвесія в проміжний буфер в стрічку
			//довжина враховує термінаторний нуль
			sprintf(b1, "%d", v1);
			//довжина враховує термінаторний нуль
			len = strlen(b1);
			pos = 3;
			//перенесення в основний буфер з кінця
			do{
				buf[pos--] = b1[--len];
			}while(len > 0); 
			buf[4] = 'm';
			buf[5] = 'V';
		break;
		//1:10
		//менші 9мВ вивід в мікровольтах			
		case 10:
			if (v1 > 99){
				sprintf(b1, "%d", v1/10);
				//довжина враховує термінаторний нуль
				len = strlen(b1);
				pos = 3;
				//перенесення в основний буфер з кінця
				do{
					buf[pos--] = b1[--len];
				}while(len > 0); 
				buf[4] = 'm';
				buf[5] = 'V';
			}
			else{
				sprintf(b1, "%d", v1*100);
				//довжина враховує термінаторний нуль
				len = strlen(b1);
				pos = 3;
				//перенесення в основний буфер з кінця
				do{
					buf[pos--] = b1[--len];
				}while(len > 0); 
				buf[4] = 'u';
				buf[5] = 'V';
			}
		break;
		//1:100
		//менші 9мВ вивід в мікровольтах			
		case 100:
			if (v1 > 990){
				sprintf(b1, "%d", v1/100);
				//довжина враховує термінаторний нуль
				len = strlen(b1);
				pos = 3;
				//перенесення в основний буфер з кінця
				do{
					buf[pos--] = b1[--len];
				}while(len > 0); 
				buf[4] = 'm';
				buf[5] = 'V';
			}
			else{
				sprintf(b1, "%d", v1*10);
				//довжина враховує термінаторний нуль
				len = strlen(b1);
				pos = 3;
				//перенесення в основний буфер з кінця
				do{
					buf[pos--] = b1[--len];
				}while(len > 0); 
				buf[4] = 'u';
				buf[5] = 'V';
			}
		break;
		//1:1000
		//все в мікровольтах
		case 1000:
			//конвесія в проміжний буфер в стрічку
			//довжина враховує термінаторний нуль
			sprintf(b1, "%d", v1);
			//довжина враховує термінаторний нуль
			len = strlen(b1);
			pos = 3;
			//перенесення в основний буфер з кінця
			do{
				buf[pos--] = b1[--len];
			}while(len > 0); 
			buf[4] = 'u';
			buf[5] = 'V';
		break;
		//0:0
		//вихід в повітрі - хз що там	
		default: 
			buf[1] = ' ';
			buf[2] = 'O';
			buf[3] = 'F';
			buf[4] = 'F';
			buf[5] = ' ';
			break;
	}
}
//-------------------------------------------------------------------
