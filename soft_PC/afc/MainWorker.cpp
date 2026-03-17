#include "MainWorker.h"
//------------------------------------------------------------------------------
extern HWND hGraph;
extern SerialPort* serialPort;
extern HWND hStatus;
extern double Vdd;
extern uint32_t userSettings[4];
std::vector<GraphControl::PointFA> pData;
std::vector<GraphControl::PointFA> pDataTemp;
//------------------------------------------------------------------------------
//тимчасові для конверсії
uint32_t freqFrom;
uint32_t freqTo;
//розрахований крок по частоті
uint16_t stepFreq;
//розрахована кількість кроків по частоті
uint16_t stepCountSet;
//------------------------
//поточна чатота для поточного пакета 0х11
uint32_t freqCur;
//номер поточного кроку по частоті
uint16_t stepCountCur;
//амплітуди на виході ген і на виході девайса та відношення voltageRatioOutIn = devOutV/genOutV
double genOutV, devOutV, voltageRatioOutIn;
//зберігання максимальної вихідної напруги
double devOutMaxV;
//накопичення амплітуди на вході для розрахунку середьої
double genOutSumV;
//значення атенюатора  1 01 100 1000 з посилки 0х11
uint16_t Att;
//------------------------------------------------------------------------------
void ShowVout();
//------------------------------------------------------------------------------
/*
char buf1[32];
sprintf_s(buf1, sizeof(buf1), "%u\r\n", n);
OutputDebugStringA(buf1);
LogFileBinary  logfile("afc_log.bin");
logfile.writeLog(buf1);
logfile.closeFile();
*/
//------------------------------------------------------------------------------

LRESULT MainWorker::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

		
		
	case WM_SERIAL_RX:
	{
		// Обробка отриманих даних з послідовного порту
		uint8_t buffer[512];
		size_t n;
		//якщо є хоч один пакет
		while (serialPort->Available() > 9) {
			n = serialPort->Read(buffer, 9);
			// Перевірка стартового байта і CRC
			if (buffer[0] != 0xA5) {
				// Невірний стартовий байт, ігноруємо пакет
				continue;
			}
			// Перевірка CRC
			if (CalculateCRC(buffer, 9) != 0) {
				// Невірний CRC, ігноруємо пакет
				continue;
			}
			// Визначаємо тип відповіді за другим байтом (ID команди)
			switch (buffer[1])
			{
			//--- Обробка відповіді на запит Vdd ---
			case 0x12:
			{
				uint8_t *buf12 = new uint8_t[9];
				memcpy(buf12, buffer, 9);
				// Відправляємо повідомлення в MainWorker для обробки даних Vdd, щоб не блокувати потік читання з порта
				PostMessage(GetHwnd(), WM_SERIAL_RX_12, 9, (LPARAM)buf12);
				break;
			}
			//є дані вимірювання
			case 0x11:
			{
				uint8_t* buf11 = new uint8_t[9];
				memcpy(buf11, buffer, 9);
				// Відправляємо повідомлення в MainWorker для обробки даних
				PostMessage(GetHwnd(), WM_SERIAL_RX_11, 9, (LPARAM)buf11);
				break;
			}




			}




		}
		return 0;









	}

		
	case WM_SERIAL_RX_11:
	{
		//uint32_t size = (uint32_t)wParam;
		uint8_t* data = (uint8_t*)lParam;
		//char buf1[64];
		//sprintf_s(buf1, sizeof(buf1), "%u\r\n", data[2]);
		//OutputDebugStringA(buf1);
		//LogFileBinary  logfile("afc_log.bin");
		//logfile.writeLogBytes(data, 9);
		//logfile.closeFile();
		//pData.push_back()
		//атенюатор на виході генератора
		switch (data[3] & 0xC0) {
			//1:1
			case 0x00:
				Att = 1;
			break;
			//1:10
			case 0x40:
				Att = 10;
			break;
			//1:100
			case 0x80:
				Att = 100;
			break;
			//1:1000
			default:
				Att = 1000;
			break;
		}
		//амплітуда на виході генератора
		genOutV = data[5] << 8 | data[4];
		//амплітуда на виході девайса
		devOutV = data[7] << 8 | data[6];
	
		//sprintf_s(buf1, sizeof(buf1), "%.0f - ", devOutV);
		//OutputDebugStringA(buf1);
		//sprintf_s(buf1, sizeof(buf1), "%.0f\r\n", Att);
		//OutputDebugStringA(buf1);
		//сумарна амплітуда
		genOutSumV += genOutV;
		//----------------------------
		//вираховуємо дані для графіку
		freqCur = userSettings[0] + userSettings[2] * stepCountCur++;
		//
		//only Vout = 0...1
		if (GraphControl::GetOnlyVout(hGraph)) {
			//
			if (devOutMaxV < devOutV)
				devOutMaxV = devOutV;
			pDataTemp.push_back(GraphControl::PointFA(freqCur, devOutV));
			//
			if (freqCur > userSettings[1]) {
				if (devOutMaxV == 0) {
					devOutMaxV = 0.1;
				}
				for (auto d : pDataTemp) {
					pData.push_back(GraphControl::PointFA(d.x, d.y / devOutMaxV));
					//sprintf_s(buf1, sizeof(buf1), "%.0f -> %.0f -> %.3f\r\n", devOutMaxV, d.y,  d.y / devOutMaxV);
					//OutputDebugStringA(buf1);
				}
				GraphControl::SetData(hGraph, pData);
				//
				ShowVout();
				//
				genOutSumV = 0;
				//!!!
				devOutMaxV = 0.001;
				//
				stepCountCur = 1;
				freqCur = userSettings[0];
				pData.clear();
				pDataTemp.clear();

			}
		}
		//Vk = (Vdev / Vgen) / Att
		else {
			voltageRatioOutIn = (devOutV / genOutV) / Att;
			pData.push_back(GraphControl::PointFA(freqCur, voltageRatioOutIn));
			//
			if (freqCur > userSettings[1]) {
				GraphControl::SetData(hGraph, pData);
				//
				ShowVout();
				//
				stepCountCur = 1;
				freqCur = userSettings[0];
				pData.clear();
			}

		}
		

		delete[] data;
		return 0;
	}

		
		
		
		
	//коли нажимаємо старт то попадаємо сюдой
    //це початковий запуск вимірювання
	case WM_SERIAL_START:
	{
		//очистка графіка від попередніх даних
		pData.clear();
		pDataTemp.clear();
		GraphControl::SetData(hGraph, pData);
		//відправити запит на вимірювання Vdd командою 0x04
		serialTxBuffer[0] = 0xA5; // Start byte
		serialTxBuffer[1] = 0x04; // get Vdd command ID
		// Дані для команди get Vdd не потрібні, тому заповнюємо 0x55
		memset(&serialTxBuffer[2], 0x55, 6);
		//
		serialTxBuffer[8] = CalculateCRC(serialTxBuffer, 8); // CRC byte
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Send 0x04 command");
		if (serialPort) {
			serialPort->Write(serialTxBuffer, sizeof(serialTxBuffer)); // Відправляємо команду в послідовний порт
		}
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Command 0x04 sent");
		userSettings[3] = 1; // Встановлюємо статус обміну даними на "запит до генератора відправлено але підтвердження немає"
		//очікуємо зміну статуса
		if (!SleepWithMessageProcessing(30, userSettings[3])) // Чекаємо до 2 секунд, поки статус не зміниться
		{
			SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"No response for Vdd request");
			//return 0; // Якщо статус не змінився, припиняємо обробку
		}
		//якщо статус змінився на 2, то продовжуємо, інакше припиняємо обробку
		//передати почакову та та кінцеву частоту командою 0x01
		serialTxBuffer[0] = 0xA5; // Start byte
		serialTxBuffer[1] = 0x01; // set frequencies command ID
		// Дані для команди set frequencies: 3 байти для частоти "From" і 3 байти для частоти "To"
		freqFrom = userSettings[0];
		freqTo = userSettings[1];
		serialTxBuffer[2] = freqFrom & 0xFF; // Частота "From" байт 0
		serialTxBuffer[3] = (freqFrom >> 8) & 0xFF; // Частота "From" байт 1
		serialTxBuffer[4] = (freqFrom >> 16) & 0xFF; // Частота "From" байт 2
		serialTxBuffer[5] = freqTo & 0xFF; // Частота "To" байт 0
		serialTxBuffer[6] = (freqTo >> 8) & 0xFF; // Частота "To" байт 1
		serialTxBuffer[7] = (freqTo >> 16) & 0xFF; // Частота "To" байт 2
		serialTxBuffer[8] = CalculateCRC(serialTxBuffer, 8); // CRC byte
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Send 0x01 command");
		if (serialPort) {
			serialPort->Write(serialTxBuffer, sizeof(serialTxBuffer)); // Відправляємо команду в послідовний порт
		}
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Command 0x01 sent");
		// Чекаємо до 2 секунд, поки статус не зміниться
		if (!SleepWithMessageProcessing(30, userSettings[3]))
		{
			SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"No response for frequencies set command");
			//return 0; // Якщо статус не змінився, припиняємо обробку
		}
		//сс=03 – старт з початкової частоти F з кроком (uint_16) 1000=0х3E8
		serialTxBuffer[0] = 0xA5; // Start byte
		serialTxBuffer[1] = 0x03; // start command ID
		freqFrom = userSettings[0];
		serialTxBuffer[2] = freqFrom & 0xFF; // Частота "From" байт 0
		serialTxBuffer[3] = (freqFrom >> 8) & 0xFF; // Частота "From" байт 1
		serialTxBuffer[4] = (freqFrom >> 16) & 0xFF; // Частота "From" байт 2
		serialTxBuffer[5] = 0x55;  //dummy data byte 1
		// Дані для команди start: 2 байти для кроку по частоті
		stepFreq = userSettings[2]; // Крок по частоті в Гц
		serialTxBuffer[6] = stepFreq & 0xFF; // Крок байт 0
		serialTxBuffer[7] = (stepFreq >> 8) & 0xFF; // Крок байт 1
		serialTxBuffer[8] = CalculateCRC(serialTxBuffer, 8); // CRC byte
		//перерахую по частотах і кроку частоти в число кроків
		stepCountSet = (userSettings[1] - userSettings[0]) / userSettings[2];
		stepCountCur = 1;
		devOutMaxV = 0;
		genOutSumV = 0;
		//send command
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Send 0x03 command");
		if (serialPort) {
			serialPort->Write(serialTxBuffer, sizeof(serialTxBuffer)); // Відправляємо команду в послідовний порт
		}
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Command 0x03 sent");
		// після відправки старту вже нічого не чекаємо
		// бо повинні почати летіти байти по прийому з АЧХ
		//OutputDebugString(L"case WM_SERIAL_START\n");
		return 0;
	}
	//коли нажимаємо стоп то попадаємо сюдой
	case WM_SERIAL_TX_07:
	{
		serialTxBuffer[0] = 0xA5; // Start byte
		serialTxBuffer[1] = 0x07; // stop command ID
		// Дані для команди stop не потрібні, тому заповнюємо 0x55
		for (int i = 2; i < 8; i++) {
			serialTxBuffer[i] = 0x55; // Data bytes
		}
		serialTxBuffer[8] = CalculateCRC(serialTxBuffer, 8); // CRC byte
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Send 0x07 command");
		if (serialPort) {
			serialPort->Write(serialTxBuffer, sizeof(serialTxBuffer)); // Відправляємо команду в послідовний порт
		}
		userSettings[3] = 1; // Встановлюємо статус обміну даними на "запит до генератора відправлено, але підтвердження немає"
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(0, 0), (LPARAM)L"Command 0x07 sent");
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)L"");
		return 0;
	}
	

	//коли отримуємо дані з порта для Vdd то попадаємо сюдой
	case WM_SERIAL_RX_12:
	{
		//uint32_t size = (uint32_t)wParam;
		uint8_t* data = (uint8_t*)lParam;
		uint32_t vddRaw = (data[5] << 24) | (data[4] << 16) | (data[3] << 8) | data[2];
		Vdd = vddRaw;// / 1000.0; //дані в мВ
		wchar_t statusText[64];
		swprintf(statusText, 64, L"Vdd: %.0f mV", Vdd);
		SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(2, 0), (LPARAM)statusText);
		userSettings[3] = 2; // Встановлюємо статус обміну даними на "запит до генератора відправлено і підтвердження є"
		delete[] data;
		return 0;
	}
	}
	//обробка
    return DefWindowProc(m_hWnd, msg, wParam, lParam);
}
//------------------------------------------------------------------------------------------------------------
void ShowVout() {
	genOutSumV *= Vdd;
	genOutSumV /= (double)stepCountCur;
	genOutSumV /= 4095;
	genOutSumV /= Att;
	wchar_t statusText[64];
	swprintf(statusText, 64, L"Vout: %.3f mV", genOutSumV);
	SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(2, 0), (LPARAM)statusText);
}
//------------------------------------------------------------------------------------------------------------

