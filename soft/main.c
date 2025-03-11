#include "stm32f30x.h"
#include "stdio.h"
#include "math.h"
#include "tgmath.h"
#include "my_global.h"
#include "TIM4\my_tim4_delay.h" 
#include "main.h"
#include "MySPI\myspi.h" 
#include "AD9833\ad9833.h" 
#include "HD44780\hd44780.h" 
#include "USART\myusart.h" 
#include "MyADC\myadc.h" 
#include "encoder\myencoder.h" 
//********************************************************************************************
//��������������� ������ � ����
/*
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
PUTCHAR_PROTOTYPE
{
	my_USART_Send((uint8_t*)(&ch));;
  return ch;
}*/
//********************************************************************************************
uint32_t GetDigitWeight(uint8_t pos);
//********************************************************************************************
extern volatile uint32_t sysTickCount;
volatile uint32_t delay_500ms;
//********************************************************************************************
const uint8_t c_generator_s[] = "AD9833 Generator\0";
const uint8_t c_kwwsoft_s[]   = "KWWSoft  (c)2025\0";
//********************************************************************************************
extern uint16_t v_att;	
//********************************************************************************************
uint8_t buf[] = "ready\r\n\0";
//������ �������  00.000.000 � ����� �� ���� [10] = 0x00;
volatile uint8_t	v_AD9833_freq_char[11];
//������� ������� �� ������ � ������ �������
//����� �� 0 �� 13. ��� 2,6, 10,11,12 �������������
//13 �������� ����� �������
volatile uint8_t 	v_fc;
//
volatile uint8_t	v_AD9833_vol_char[7];   //[6] = 0x00
//   2345mV
//********************************************************************************************
uint32_t v1;
//������ ��������� - ��������� �� ����������. ������� ������
uint8_t joy_button;
uint8_t enc_button;
//********************************************************************************************

int main(void){
 	//GPIO_InitTypeDef port;
	//1ms
	SysTick_Config((SystemCoreClock / 1000) - 1);
	//������� ������
	delay_TIM4_init();
	HD44780_Init();
	HD44780_Clear();
	HD44780_Out_String(0, 0, (uint8_t*)c_generator_s);
	HD44780_Out_String(1, 0, (uint8_t*)c_kwwsoft_s);
	delay_TIM4_ms(1000);
	//�������� ������
	my_SPI_Init();
	AD9833_Init();
	my_Encoder_Init();
	my_ADC_Init();
	my_ADC2_Vref_Init();
	my_USART_Init();
	//����������� ��� ������ �� ���������
	//��������� 1000�� RMS
	v_MCP1410_volume = 211;
	v_enc_percent = (v_MCP1410_volume * 100) / 255;
	v_AD9833_freq = 465000;
	v_AD9833_type = AD9833_CMD_SINE;
	//������ �� ������ �� �����
	v_fc = 5;
	v_AD9833_vol_char[6] = 0;
	//�������� �� ������ ������� ����
	joy_button = 0x00;
	//������ �������� - ����� ����
	//0x01 - button presed
	//0x10 - modulation ON
	enc_button = 0x00;
	//
	my_Encoder_Set(v_MCP1410_volume);
	MCP1410_SetVolume(v_MCP1410_volume);
	AD9833_SetFrequency(v_AD9833_freq, v_AD9833_type);
	//AD9833_SetType(v_AD9833_type);
	//
	//delay_TIM4_ms(2000);
	HD44780_Clear();
	//��� ���� �� �����
	AD9833_ShowType(v_AD9833_type);
	//�������� � ��������
	MCP1410_ShowVolume(v_enc_percent);
	//������� � ������ ������������ � ������ �� ������ � ��������
	v_AD9833_freq_char[10] = 0x00;
	AD9833_MakeFreqString(v_AD9833_freq, (uint8_t*)v_AD9833_freq_char);
	HD44780_Out_String(0, 0, (uint8_t*)v_AD9833_freq_char);
	//�������� ������ �� ��c���
	HD44780_Set_Pos(0, v_fc);
	HD44780_Set_Cursor(2);
	//
/*	
	printf("systick: %d\r\n", sysTickCount);

	printf("vref: %d\r\n", my_ADC2_Vref_Get());
	printf("calib: %d\r\n", V_ref_cal);

	printf("systick: %d\r\n", sysTickCount);
	*/	
  /* Infinite loop */
	uint8_t v2 = 150;
	//500 ms
	delay_500ms = sysTickCount + 500;
  while (1){
		//���������� ������ ���������
		//�������� ��� �������
		//���� ����� - ������� ��� ����� ������� 
		if (joy_button == 0){
			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == Bit_RESET){
				joy_button = 0xff;
			}
		}
		//������ ��������
		//��������� �� ��������� ���������
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET){
			//������ �� �� ���� ���������
			enc_button |= 0x01;
		}
		//����� ������� �� �����
		v1 = my_Encoder_Get();
		if (v1 != enc_static){
			v_MCP1410_volume = v1;
			v_enc_percent = (v_MCP1410_volume * 100) / 255;
			MCP1410_SetVolume(v_MCP1410_volume);
			MCP1410_ShowVolume(v_enc_percent);
			HD44780_Set_Pos(0, v_fc);
		}
		//
		//������� �� ����� �������
		//������ ��� ����������� ������
		//�������� �� ����
		//��������� ��������� ����������
		//�������� �� ����� ���������
		v1 = my_ADC_GetJoystick();
				//printf("var1: %d\r\n", v_att);
		switch(v1){
			//���� �� ������
			case joy_left:
//				printf("var1: %d\r\n", v1);
				//�� ����� ��� ���������
				if (v_fc != 0){
					//��������� ����
					v_fc --;
					//���� ������ �� ���� �� ������� �� 9
					if (v_fc == 12)
						v_fc = 9;
					//���� ���� �� ������ �� �� �� 1 ����
					if ( (v_fc == 6) || (v_fc == 2) ){
						v_fc --;
					}
				}
				//���������� ������ �� �����
				HD44780_Set_Pos(0, v_fc);
				break;
			//������ �� ������	
			case joy_right:
//				printf("var1: %d\r\n", v1);
				//�� ����� ����� ���������
				if (v_fc != 13){
					//��������� ������
					v_fc ++;
					//���� ������ 10 �� ������� �� ����� �������
					if (v_fc == 10)
						v_fc = 13;
					//���� ���� �� ������ �� �� �� 1 ������
					if ( (v_fc == 6) || (v_fc == 2) ){
						v_fc ++;
					}
				}
				//���������� ������ �� �����
				HD44780_Set_Pos(0, v_fc);
				break;
			//�������� ������� �� ����� ������� ������� �� �������
			//��� ������ ����� �������
			case joy_up:
//				printf("var1: %d\r\n", v1);
				if (v_fc == 13){
					if (v_AD9833_type == AD9833_CMD_SINE){
						v_AD9833_type = AD9833_CMD_SQUARE;
					}
					else if (v_AD9833_type == AD9833_CMD_SQUARE){
						v_AD9833_type = AD9833_CMD_TRIANGLE;
					}
					else{
						v_AD9833_type = AD9833_CMD_SINE;
					}
					AD9833_SetType(v_AD9833_type);
					AD9833_ShowType(v_AD9833_type);
				}
				//�������
				else{
					//���� ����������� �������
					v1 = GetDigitWeight(v_fc);
					//���� � �������� ������� ������� �� �������� �� ������ ����
					if ( !((v_AD9833_freq + v1) > 12500000) ){
						//�������� ������� �� ������� �������
						v_AD9833_freq += v1;
						//���������� ������� � �����
						AD9833_MakeFreqString(v_AD9833_freq, (uint8_t*)v_AD9833_freq_char);
						//�������� �� ����� ��� ������
						HD44780_Out_String(0, 0, (uint8_t*)v_AD9833_freq_char);
						//��������� ������� �� ����
						HD44780_Set_Pos(0, v_fc);
						//���������� ������� �� �����
						AD9833_SetFrequency(v_AD9833_freq, v_AD9833_type);
					}
				}
				break;
			//�������� ������� �� ����� ������� ������� �� �������
			case joy_down:
//				printf("var1: %d\r\n", v1);
				//�������
				if (v_fc == 13){
				}
				//�������
				else{
					//���� ����������� �������
					v1 = GetDigitWeight(v_fc);
					//���� ���������� ������� �� �������� ��� �� ����
					if ( v_AD9833_freq > v1 ){
							//�������� ������� �� ������� �������
							v_AD9833_freq -= v1;
						//���������� ������� � �����
						AD9833_MakeFreqString(v_AD9833_freq, (uint8_t*)v_AD9833_freq_char);
						//�������� �� ����� ��� ������
						HD44780_Out_String(0, 0, (uint8_t*)v_AD9833_freq_char);
						//��������� ������� �� ����
						HD44780_Set_Pos(0, v_fc);
						//���������� ������� �� �����
						AD9833_SetFrequency(v_AD9833_freq, v_AD9833_type);
					}
					//������� ������ 0 - ������� � ����� � ����� ����������
					//������ ������ �� �������� � ������� �������
					else if (v_AD9833_freq != 0){
						//�������� ���� �� ��������
						do{
							//��������� ������
							v_fc ++;
							//�� ������ ������� �� ����
							if (v_fc == 10){
								v_fc = 9;
								break;
							}
							//���� ���� �� ������ �� �� �� 1 ������
							if ( (v_fc == 6) || (v_fc == 2) ){
								v_fc ++;
							}
							//���� ����������� �������
							v1 = GetDigitWeight(v_fc);
							//���� ���������� ������� �� �������� ��� �� ����
							if ( v_AD9833_freq > v1 ){
									//�������� ������� �� ������� �������
									v_AD9833_freq -= v1;
								//���������� ������� � �����
								AD9833_MakeFreqString(v_AD9833_freq, (uint8_t*)v_AD9833_freq_char);
								//�������� �� ����� ��� ������
								HD44780_Out_String(0, 0, (uint8_t*)v_AD9833_freq_char);
								//��������� ������� �� ����
								HD44780_Set_Pos(0, v_fc);
								//���������� ������� �� �����
								AD9833_SetFrequency(v_AD9833_freq, v_AD9833_type);
								break;
							}
						}while(v_fc > 9);
					}
				}
				break;
			default:
				break;
		}
		//���� ������� ��������� ���� �� 2 ��������� � ���
		//��������� ������� �� �������� �� ����� � ���������
		//�������� 
		//���� ��������� �� �������� - ��� �� ����� ������
		if ( (sysTickCount > delay_500ms) & ((enc_button & 0x10) == 0x00)){
			//�������� �������� �������� �� �����
			delay_500ms = sysTickCount + 800; 
			//����� ��������
			//��� ���������� ������� �������� �� ������
			//����� ����� ��� � ������������ ����������
			my_ADC_MakeVolumeString((uint8_t*)v_AD9833_vol_char);
   		//�������� �� ����� ��� ������
			HD44780_Out_String(1, 5, (uint8_t*)v_AD9833_vol_char);
			//��������� ������� �� ����
			HD44780_Set_Pos(0, v_fc);
			//����� ������� �� ������ ���������
			if (joy_button != 0){
				if (v_AD9833_type == AD9833_CMD_SINE){
					v_AD9833_type = AD9833_CMD_SQUARE;
				}
				else if (v_AD9833_type == AD9833_CMD_SQUARE){
					v_AD9833_type = AD9833_CMD_TRIANGLE;
				}
				else{
					v_AD9833_type = AD9833_CMD_SINE;
				}
				AD9833_SetType(v_AD9833_type);
				AD9833_ShowType(v_AD9833_type);
				//������� ������� ������
				joy_button = 0;
		  }
			//printf("value: %d\r\n", v_att);
		}
		//��������� --------------------------------
		//������� ������� ���������/��������� ���������
		//���� ��������� �������� - �� ���������
		if ((enc_button & 0x01) == 0x01){
			//������� �� ���������
			enc_button &= ~0x01;
			//������ �� ��������
			delay_TIM4_ms(300);
			//��������� ���� �������� - ��������
			if ((enc_button & 0x10) == 0x10){
				enc_button &= ~0x10;
				HD44780_Set_Pos(1, 0);
				HD44780_Out_String(1, 0, (uint8_t*)" ");
				MCP1410_SetVolume(v_MCP1410_volume);
				MCP1410_ShowVolume(v_enc_percent);
				HD44780_Set_Pos(0, v_fc);
			}
			//�������� ��������� ���������
			else{
				//�� ������ �������� ���������
				enc_button |= 0x10;
				//�� ������� ������ ���������
				HD44780_Out_String(1, 0, (uint8_t*)"M");
			}
			//��������� ������� �� ����
			HD44780_Set_Pos(0, v_fc);
		}
	//��������� �������� - ���������� ��������� ������ 0...90 degree
		//(�����+1)/2
		//0.5 0.59 0.67 0.75 0.82 0.88 0.93 0.97 0.99 1.0
		//������ 1000��
		if ((enc_button & 0x10) == 0x10){
			MCP1410_SetVolume(200*0.5);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.59);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.67);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.75);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.82);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.88);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.93);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.99);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.99);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.93);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.88);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.82);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.75);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.67);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.59);
			delay_TIM4_us(50);
			MCP1410_SetVolume(200*0.5);
			delay_TIM4_us(50);
		}
		//��������� ����� --------------------------------
  }
}
//********************************************************************************************
uint32_t GetDigitWeight(uint8_t pos){
	switch(pos){
		case 9: return 1;
		case 8: return 10;
		case 7: return 100;
		case 5: return 1000;
		case 4: return 10000;
		case 3: return 100000;
		case 1: return 1000000;
		case 0: return 10000000;
	}
	return 0;
}
//********************************************************************************************
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif



