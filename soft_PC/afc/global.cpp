#include "global.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// --- Function to calculate CRC8 checksum for a given data buffer and length, returns the calculated CRC value ---
uint8_t CalculateCRC(const uint8_t* pdata, size_t len)
{
    /*
    розрахунок контрольної суми по алгоритму црц8
    рахуються байти переданої довжини
    на передачу рахувати потрібно до самого байта кс але без нього
    по прийому рахувати з байтом кс - повинно бути 0
    */
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t data = pdata[i];
        for (int j = 8; j > 0; j--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }
    return crc;
}
//------------------------------------------------------------------------
/*
auto ports = EnumerateComPorts();
for (const auto& p : ports){
    wprintf(L"%s -> %s\n", p.first.c_str(), p.second.c_str());
}
*/
// //enumerate com ports
std::vector<std::pair<std::wstring, std::wstring>> EnumerateComPorts()
{
    
    std::vector<std::pair<std::wstring, std::wstring>> result;

    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_PORTS,
        nullptr,
        nullptr,
        DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE)
        return result;

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i)
    {
        WCHAR friendlyName[256];
        DWORD requiredSize = 0;

        if (!SetupDiGetDeviceRegistryPropertyW(
            hDevInfo,
            &devInfoData,
            SPDRP_FRIENDLYNAME,
            nullptr,
            reinterpret_cast<PBYTE>(friendlyName),
            sizeof(friendlyName),
            &requiredSize))
        {
            continue;
        }

        std::wstring name = friendlyName;

        // Шукаємо "(COMx)"
        size_t pos = name.find(L"(COM");
        if (pos == std::wstring::npos)
            continue;

        size_t end = name.find(L")", pos);
        if (end == std::wstring::npos)
            continue;

        std::wstring comNumber = name.substr(pos + 1, end - pos - 1); // COM3

        result.emplace_back(comNumber, name);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return result;
}
//------------------------------------------------------------------------------
//затримку очікування з можливістю обробки повідомлень, щоб не блокувати інтерфейс під час очікування відповіді від генератора
bool SleepWithMessageProcessing(DWORD milliseconds, uint32_t flag)
{
    DWORD startTime = GetTickCount();
    while (GetTickCount() - startTime < milliseconds)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1); // Невелика пауза для зменшення навантаження на CPU
        if (flag != 1)// Якщо статус змінився, припиняємо очікування
            return true;
    }
    return false; // Час очікування вичерпано	
}
//------------------------------------------------------------------------------
//Unicode → UTF-8
std::string WideToUtf8(const wchar_t* wstr)
{
    if (!wstr) return {};

    int sizeNeeded = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr);

    if (sizeNeeded <= 0)
        return {};

    std::string result(sizeNeeded - 1, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        result.data(),
        sizeNeeded,
        nullptr,
        nullptr);

    return result;
}
// //--------------------------------------------------------------
//Unicode → ANSI
std::string WideToANSI(const wchar_t* wstr)
{
    if (!wstr) return {};

    int sizeNeeded = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr);

    if (sizeNeeded <= 0)
        return {};

    std::string result(sizeNeeded - 1, 0);

    WideCharToMultiByte(
        CP_ACP,
        0,
        wstr,
        -1,
        result.data(),
        sizeNeeded,
        nullptr,
        nullptr);

    return result;
}
// //--------------------------------------------------------------
