#include "stm32f30x.h"
#include "stdio.h"
#include "string.h"
#include "my_global.h"
#include "USART\myusart.h" 
#include "..\TIM4\my_tim4_delay.h" 
#include "myadc.h"
//-------------------------------------------------------------------
#define VREFINT_CAL_ADDR  ((uint16_t*)0x1FFFF7BA)

#define TS_CAL1_ADDR      ((uint16_t*)0x1FFFF7B8) // 30°C
#define TS_CAL2_ADDR      ((uint16_t*)0x1FFFF7C2) // 110°C

#define VREFINT_CAL       (*VREFINT_CAL_ADDR)
#define TS_CAL1           (*TS_CAL1_ADDR)
#define TS_CAL2           (*TS_CAL2_ADDR)
//-------------------------------------------------------------------
#define PUTCHAR_PROTOTYPE1 int fputc(int ch, FILE *f)
/*
PUTCHAR_PROTOTYPE1
{
	my_USART1_Send((uint8_t*)(&ch));;
  return ch;
}
*/
//-------------------------------------------------------------------
//-------------------------------------------------------------------
extern volatile uint32_t sysTickCount;
static volatile uint32_t sysTickInterval;
//-------------------------------------------------------------------
uint16_t v_x, v_y;	
uint16_t v_att;	
uint16_t v_out_adc;	
//uint8_t	flag_adc; //0x00 - data not ready
//-------------------------------------------------------------------
void my_ADC_Init(){
  GPIO_InitTypeDef port;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InjectedInitTypeDef adc;
	//NVIC_InitTypeDef  NVIC_InitStruct;
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

    // 1. Тактування (ADC12 використовують спільну шину)
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
    // Налаштування джерела тактування ADC (PLL)
    RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);

    // 2. Активація внутрішнього регулятора напруги ADC
    // Це критично для F3 серії перед калібруванням
    ADC_VoltageRegulatorCmd(ADC1, ENABLE);
    // Затримка для стабілізації регулятора (мінімум 10 мкс)
		delay_TIM4_us(30);

    // 3. Процедура калібрування
    // Вибираємо Single-ended режим (найчастіший випадок)
    ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
    ADC_StartCalibration(ADC1);
    
    // Чекаємо завершення калібрування
    while(ADC_GetCalibrationStatus(ADC1) != RESET);
    
    // Отримання калібрувального значення (необов'язково, але корисно для дебагу)
    // uint32_t calibration_value = ADC_GetCalibrationValue(ADC1);

    // 4. Загальні налаштування (Common)
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
    ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
    ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
    ADC_CommonInit(ADC1, &ADC_CommonInitStructure);

    // 5. Конфігурація самого ADC1
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
    ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
    ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
    //ADC_InitStructure.ADC_AutoDelay = ADC_AutoDelay_Disable;
    ADC_InitStructure.ADC_NbrOfRegChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // 6. Увімкнення внутрішніх каналів (Vref & Temp)
    ADC_VrefintCmd(ADC1, ENABLE);
    //ADC_TempSensorCmd(ADC1, ENABLE);


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
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_1, ADC_SampleTime_61Cycles5);
	//джойстик вісь y
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_2, ADC_SampleTime_61Cycles5);
	//положення перемикача атенюатора
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_3, ADC_SampleTime_61Cycles5);
	//напруга на виході генератора з детектора
	ADC_InjectedChannelSampleTimeConfig(ADC1, ADC_InjectedChannel_4, ADC_SampleTime_61Cycles5);
	// Увімкнення інжекторного режиму
	ADC_AutoInjectedConvCmd(ADC1, ENABLE);
	//преривання по завершенню перетворення
	//ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);
	
	//allow interrupts 
//  NVIC_InitStruct.NVIC_IRQChannel = ADC1_IRQn;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 6;
//  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);


// 7. Остаточне увімкнення ADC
    ADC_Cmd(ADC1, ENABLE);

    // Чекаємо прапорця готовності (ADRDY)
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
		//
		//flag_adc = 0xff;
}
	
//-------------------------------------------------------------------
uint32_t my_Read_Vdda_mV(void) {
    uint32_t sum_vref = 0;
		uint32_t vref_data = 0;
		// Налаштовуємо канал 18 (Vrefint) для ADC1
    // SampleTime має бути великим для Vrefint (мін. 17.1 мкс за даташитом)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_601Cycles5);

		for (uint8_t i = 0; i < 8; i++){
			ADC_StartConversion(ADC1);
			while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET){};
      sum_vref += ADC_GetConversionValue(ADC1);
		}
		vref_data = sum_vref / 8;
    // Розрахунок: (3300 * заводське_значення) / поточне_значення
    uint32_t vdda = (3300 * (*VREFINT_CAL_ADDR)) / vref_data;
    return vdda;
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
	uint32_t v_Vdda;
	uint32_t v2;
	//напруга
	uint32_t v1;
	//взяти опорне
	v_Vdda = my_Read_Vdda_mV();
	//напруга на виході детектора в мілівольтах
	v2 = (v_Vdda * v_out_adc) / 4095;
//	printf("v_det: %d\r\n", v1);
	//вибираємо похибку по рівнянню прямої
	//по результатах калібровки
	//1:1 1:1000
	v1 = (uint32_t)(v2 * 0.7922 + 64.9754);
	//v1 = v2;
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
			//вибираємо похибку по рівнянню прямої
			//по результатах калібровки
			//1:10
			v1 = (uint32_t)((v2/10.0 * 0.7312 + 5.3487) * 10.0);
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
			//вибираємо похибку по рівнянню прямої
			//по результатах калібровки
			//1:100
			v1 = (uint32_t)((v2/100.0 * 0.702 + 0.413) * 100.0);
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
