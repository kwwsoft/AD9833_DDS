#pragma once
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>
#include <thread>
#include <atomic>
#include <vector>
#include "framework.h"
#include <commctrl.h>
#include <windowsx.h>
#include <string>
//загальні процедури і примочки , які не мають конкретного класу
//----------------------------------------------------------------
// --- Function to calculate CRC8 checksum for a given data buffer and length, returns the calculated CRC value ---
uint8_t CalculateCRC(const uint8_t* pdata, size_t len);
// //enumerate com ports
std::vector<std::pair<std::wstring, std::wstring>> EnumerateComPorts();
//затримку очікування з можливістю обробки повідомлень, щоб не блокувати інтерфейс під час очікування відповіді від генератора
bool SleepWithMessageProcessing(DWORD milliseconds, uint32_t &flag);
//
std::string WideToUtf8(const wchar_t* wstr);
std::string WideToANSI(const wchar_t* wstr);
