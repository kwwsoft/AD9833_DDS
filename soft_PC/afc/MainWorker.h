#pragma once
#include "framework.h"
#include <commctrl.h>
#include <windowsx.h>
#include <Windows.h>
#include <string>
#include "resource.h"
#include "global.h"
#include "LogFileBinary.h"
#include "EditFreqSubClass.h"
#include "GraphControl.h"
#include "RingBuffer.h"
#include "SerialPort.h"

//клас для прийому та обробки повідомлень з головного вікна та послідовного порта
//створює приховане вікно для отримання повідомлень
class MainWorker
{
public:
	MainWorker(HINSTANCE hInst) {
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = StaticWndProc;
		wc.hInstance = hInst;
		wc.lpszClassName = L"MainWorkerWindowClass";
		RegisterClassEx(&wc);
		// Create a hidden message-only window
		m_hWnd = CreateWindowEx(0, wc.lpszClassName, L"MainWorkerHiddenWindow", 0,
			0, 0, 0, 0, 
			//ознака для створення прихованого вікна, яке не відображається на екрані і не отримує фокус
			HWND_MESSAGE, 
			nullptr, hInst, this);
	}
	~MainWorker() {
		if (m_hWnd) {
			DestroyWindow(m_hWnd);
		}
	}
	// Method to get the hidden window handle for message posting
	HWND GetHwnd() const { return m_hWnd; }

private:
	HWND m_hWnd;
	// Static window procedure to route messages to the instance method
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		MainWorker* pThis = nullptr;

		if (msg == WM_NCCREATE)
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
			pThis = (MainWorker*)cs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
			pThis->m_hWnd = hWnd;
		}
		else
		{
			pThis = (MainWorker*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis)
			return pThis->WndProc(msg, wParam, lParam);

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	// Instance window procedure to handle messages
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	//variable
private:
	uint8_t serialRxBuffer[1024]; // буфер для прийому даних з послідовного порта
	uint8_t serialTxBuffer[9]; // буфер для відправки даних в послідовний порт
};

