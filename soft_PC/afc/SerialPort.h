#pragma once
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
//#include <regstr.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>
#include <thread>
#include <atomic>
#include <vector>
#include "MainWorker.h"
#include "LogFileBinary.h"
#include "resource.h"
#include "RingBuffer.h"
//----------------------------------------------------------------------------------
#pragma comment(lib, "setupapi.lib")
//----------------------------------------------------------------------------------
//до класу не належать - але потрібні в роботі
//----------------------------------------------------------------------------------
// --- Function to calculate CRC8 checksum for a given data buffer and length, returns the calculated CRC value ---
//uint8_t CalculateCRC(const uint8_t* pdata, size_t len);
//------------------------------------------------------
// //enumerate com ports
//std::vector<std::pair<std::wstring, std::wstring>> EnumerateComPorts();
//auto ports = EnumerateComPorts();
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// --- Class to manage serial port communication, including opening, writing, reading, and closing the port ---
class SerialPort
{
public:
	//конструктор
    SerialPort(HWND hwnd, size_t rxBufferSize) : hWnd(hwnd), rxBuffer(rxBufferSize) {}
    
    //деструктор
    ~SerialPort(){Close();};

    //закрити порт
    void Close();
   
    //відкрити порт
    bool Open(const std::wstring& port, DWORD baud = CBR_115200);
    
    // скільки байт є в буфері для читання
    size_t Available() const;
	
    //прочитати дані з буфера, повертає кількість фактично прочитаних байтів
    size_t Read(uint8_t* data, size_t len);

    //прочитати дані з буфера без видалення їх, повертає кількість фактично прочитаних байтів
    size_t Peek(uint8_t* data, size_t len) const;

    // записати дані в порт, повертає кількість фактично записаних байтів
	size_t Write(const uint8_t* data, size_t len);

    //повернути хендл порта
    HWND GetHwnd() const{
        return hWnd;
    }

    //повернути статус порта
    bool GetStatus() const {
        return running;
    }

private:
    void RxWorker();

    HWND hWnd = nullptr;
	HANDLE hPort = INVALID_HANDLE_VALUE;
    
    std::atomic<bool> running = false;
    std::thread rxThread;
  
    RingBuffer rxBuffer;

};
//------------------------------------------------------------------
