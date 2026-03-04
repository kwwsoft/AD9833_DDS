#include "stm32f30x.h"                  // Device header
#include "myafc.h"
//********************************************************************************************
extern uint16_t v_x, v_y;	
extern uint16_t v_att;	
extern uint16_t v_out_adc;	
//********************************************************************************************
uint8_t rxBuf[9];
uint8_t txBuf[9];
uint8_t rxBufPos;
uint32_t Vdda;
//задані апраметри качання
uint32_t freqFrom;
uint32_t freqTo;
//крок по частоті в герцах
uint32_t freqStep;
//поточні параметри качання
uint32_t freqCur;
uint32_t step;
//work = 1, stop = 0;
uint8_t WorkStatus;
//********************************************************************************************
void my_AFC_Run(void){
	//init
	memset(rxBuf, 0, 9);
	rxBufPos = 0;
	my_USART1_Init();
	my_USART1_DMA_RX_Init();
	//my_USART1_DMA_TX_Init();
	HD44780_Out_String(0, 0, (uint8_t*)"AFC wait command");
	//main loop
	//з ввімкненою оптимізацією не працює
	//і while(1) теж
	label1:;
		if (rxBuf[1] == 0){
		}
		//сс=01 – передача початкової та кінцевої частоти F (uint_32) 12500000=0хBEBC20
		else if (rxBuf[1] == 0x01){
			//забрати частоту з посилки
			freqFrom = rxBuf[4]<<16 | rxBuf[3]<<8 | rxBuf[2];
			freqTo = rxBuf[7]<<16 | rxBuf[6]<<8 | rxBuf[5];
			//показати частоти на дисплей
			AD9833_MakeFreqString(freqFrom, (uint8_t*)v_AD9833_freq_char);
			HD44780_Clear();
			HD44780_Out_String(0, 0, (uint8_t*)v_AD9833_freq_char);
			AD9833_MakeFreqString(freqTo, (uint8_t*)v_AD9833_freq_char);
			HD44780_Out_String(1, 0, (uint8_t*)v_AD9833_freq_char);
			//скинути і обнулити буфер прийому
			rxBufPos = 0;
			memset(rxBuf, 0, 9);
		}


		//сс=03 – старт з початкової частоти F з кроком (uint_16) 1000=0х3E8
		//основний цикл вимірювання і відправки даних
		else if (rxBuf[1] == 0x03){
			freqStep = rxBuf[7]<<8 | rxBuf[6];
			WorkStatus = 1;
			step = 0;
			freqCur = freqFrom;
			rxBufPos = 0;
			memset(rxBuf, 0, 9);
			//основний потік відпраки даних
			while(WorkStatus == 1){
				AD9833_SetFrequency(freqCur, AD9833_CMD_SINE);
				delay_TIM4_us(100);
				my_ADC_Start();
				txBuf[0] = 0xA5;
				txBuf[1] = 0x11;
				//step
				txBuf[2] = step & 0x00FF;
				txBuf[3] = (step & 0x3F00)>>8;
				//att
				switch(GetAttValue()){
					case 1:
					break;
					case 10:
						txBuf[3] |= 0b01000000;
					break;
					case 100:
						txBuf[3] |= 0b10000000;
					break;
					case 1000:
						txBuf[3] |= 0b11000000;
					break;
				}
				//out gen
				txBuf[4] = v_out_adc & 0x00FF;
				txBuf[5] = (v_out_adc & 0xFF00)>>8;
				//out device
				txBuf[6] = v_x & 0x00FF;
				txBuf[7] = (v_x & 0xFF00)>>8;
				//crc
				txBuf[8] = CalculateCRC(txBuf, 8);
				my_USART1_Send(txBuf, 9);
				//
				step++;
				freqCur += freqStep;
				//reset to begin
				if (freqCur > freqTo){
					step = 0;
					freqCur = freqFrom;
					delay_TIM4_ms(500);
					//break;
				}
				rxBufPos = 0;
				memset(rxBuf, 0, 9);
			}
			//HD44780_Clear();
			//HD44780_Out_String(0, 0, (uint8_t*)"AFC wait command");
		}


		//сс=04 – запит напруги живлення генератора
		else if (rxBuf[1] == 0x04){
			Vdda= my_Read_Vdda_mV();
			txBuf[0] = 0xA5;
			txBuf[1] = 0x12;
			txBuf[2] = Vdda & 0x000000FF;
			txBuf[3] = (Vdda & 0x0000FF00)>>8;
			txBuf[4] = (Vdda & 0x00FF0000)>>16;
			txBuf[5] = (Vdda & 0xFF000000)>>24;
			txBuf[6] = 0x55;
			txBuf[7] = 0x55;
			txBuf[8] = CalculateCRC(txBuf, 8);
			my_USART1_Send(txBuf, 9);
			HD44780_Out_String(1, 0, (uint8_t*)"Receive 0x04");
			rxBufPos = 0;
			memset(rxBuf, 0, 9);
		}
		else{
		}
goto label1;
	//hz....
	while(1){};
}
//********************************************************************************************
void my_AFC_ProcessData(uint8_t *data, uint16_t len){
	//ще не було нічого прийнято
	if (rxBufPos == 0){
		//копіюємо інфу з буферу ДМА в локальний
		memcpy(rxBuf, data, len);
		//пошук початку пакета
		do{
			if (rxBuf[rxBufPos] == 0xA5)
				break;
		}while(rxBufPos++ < len);
		//старт пакета не знайшов 
		if (rxBufPos > len)
			return;
		//старт пакета є
		//пакет з нульового байта
		if (rxBufPos == 0){
				//контрольна сума правильна
				if (CalculateCRC(rxBuf, 9) == 0){
					//зупинку вимірювання обробити тут - інакше ніяк
					//сс=07 – стоп вимірювання
					if (rxBuf[1] == 0x07){
						WorkStatus = 0;
						rxBufPos = 0;
						memset(rxBuf, 0, 9);
					}
					return;
				}
				//відкидаємо пакет
				else{
					rxBufPos = 0;
					return;
				}
		}
	
	}
}
//********************************************************************************************

