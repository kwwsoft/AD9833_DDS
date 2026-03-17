#include "global.h"
#include "SerialPort.h"
//------------------------------------------------------------------
extern HWND hStatus;                         // status bar
extern MainWorker* mainWorkerPtr;
//------------------------------------------------------------------

//------------------------------------------------------------------------------
// --- Method to close the serial port and clean up resources, including stopping the receive thread and closing the port handle ---
void SerialPort::Close()
{
    if (!running)
        return;

    running = false;

    PurgeComm(hPort, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT);
    CancelIo(hPort);

    if (rxThread.joinable())
        rxThread.join();

    CloseHandle(hPort);
    hPort = INVALID_HANDLE_VALUE;
}

// --- Method to open the serial port with specified port name and baud rate, configure it, and start the receive thread ---
bool SerialPort::Open(const std::wstring& port, DWORD baud)
{
    hPort = CreateFileW(
        port.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,  // БЕЗ overlapped
        nullptr);

    if (hPort == INVALID_HANDLE_VALUE)
        return false;

    SetupComm(hPort, 65536, 32768);
    PurgeComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);

    DCB dcb = {};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(hPort, &dcb);

    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fBinary = TRUE;
    dcb.fOutxCtsFlow = FALSE;               //выключаем режим слежения за сигналом CTS
    dcb.fOutxDsrFlow = FALSE;               //выключаем режим слежения за сигналом DSR
    dcb.fDtrControl = DTR_CONTROL_ENABLE;  //отключаем использование линии DTR
    dcb.fDsrSensitivity = FALSE;            //отключаем восприимчивость драйвера к состоянию линии DSR
    dcb.fNull = FALSE;                      //разрешить приём нулевых байтов
    dcb.fRtsControl = RTS_CONTROL_ENABLE;  //отключаем использование линии RTS
    //dcb.fAbortOnError = FALSE;              //отключаем остановку всех операций чтения/записи при ошибке

    if (!SetCommState(hPort, &dcb))
        return false;

    COMMTIMEOUTS t = {};
    t.ReadIntervalTimeout = 50;
    t.ReadTotalTimeoutConstant = 50;
    t.ReadTotalTimeoutMultiplier = 10;
    t.WriteTotalTimeoutConstant = 5;
    t.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(hPort, &t);

    running = true;
    rxThread = std::thread(&SerialPort::RxWorker, this);

    return true;
}
//скільки байт можна прочитати
size_t SerialPort::Available() const
{
    return rxBuffer.Available();
}

//прочитати дані з буфера, повертає кількість фактично прочитаних байтів
size_t SerialPort::Read(uint8_t* data, size_t len)
{
    return rxBuffer.Read(data, len);
}

//прочитати дані з буфера без видалення їх, повертає кількість фактично прочитаних байтів
size_t SerialPort::Peek(uint8_t* data, size_t len) const
{
    return rxBuffer.Peek(data, len);
}

// --- Method that runs in a separate thread to continuously read data from the serial port, process it, and store it in the ring buffer ---
void SerialPort::RxWorker()
{
    uint8_t buffer[128]{ 0 };

    while (running)
    {
        DWORD bytesRead = 0;

        if (ReadFile(hPort, buffer, sizeof(buffer), &bytesRead, nullptr))
        {
            if (bytesRead > 0)
            {
                //SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)L"COM5 read");
                //LogFileBinary  logfile("afc_log.bin");
                //logfile.writeLogBytes(buffer, bytesRead);
                //logfile.closeFile();
				//забираємо дані в кільцевий буфер
                rxBuffer.Write(buffer, bytesRead);
				//повідомляємо головне вікно, що дані прийшли і їх можна читати з буфера
				//WParam - кількість байт, які можна читати з буфера
				//LParam - скільки байт записали в буфер, можна використовувати для відладки
				//SendMessage(hWnd, WM_SERIAL_RX, rxBuffer.Available(), bytesRead);
                SendMessage(mainWorkerPtr->GetHwnd(), WM_SERIAL_RX, rxBuffer.Available(), bytesRead);
            }
        }
    }
}

/// --- Method to write data to the serial port, returns the number of bytes actually written ---
size_t SerialPort::Write(const uint8_t* data, size_t len)
{
    if (!running)
        return 0;
    DWORD bytesWritten = 0;
    if (!WriteFile(hPort, data, static_cast<DWORD>(len), &bytesWritten, nullptr))
        return 0;
    //FlushFileBuffers(hPort);
    return bytesWritten;

}

