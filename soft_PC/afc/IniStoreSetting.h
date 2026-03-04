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
#include <codecvt>
#include "resource.h"
//--------------------------------------------
//для списку портів в системі і індекс в меню вибраного порту
extern std::vector<std::pair<std::wstring, std::wstring>> g_ComPorts;
extern int g_SelectedPort;
extern HMENU g_hComMenu;
//--------------------------------------------

class IniStoreSetting
{
public:
    void static SaveSelectedPort(const std::wstring& portName);
    void static SaveFrequencyFrom(const std::wstring& value);
    void static SaveFrequencyFrom(const std::string& value);
    void static SaveFrequencyTo(const std::wstring& value);
    void static SaveFrequencyTo(const std::string& value);
    void static SaveFrequencyStep(const std::wstring& value);
    void static SaveFrequencyStep(const std::string& value);
    void static SaveOnlyVout(const std::wstring& value);
    void static SaveOnlyVout(const std::string& value);
    std::wstring static LoadSelectedPort();
    std::wstring static LoadFrequencyFrom();
    std::wstring static LoadFrequencyTo();
    std::wstring static LoadFrequencyStep();
    std::wstring static LoadOnlyVout();
    void static RestoreSelectedPort();
    static std::wstring GetPortName();
    static std::wstring GetPortNameLong();
    static void SetPortName(const std::wstring& value);
    static void SetPortNameLong(const std::wstring& value);
    static bool GetPortAvaliable();
  
private:
    IniStoreSetting() = delete;
    ~IniStoreSetting() = delete;
    IniStoreSetting(const IniStoreSetting&) = delete;
    IniStoreSetting& operator=(const IniStoreSetting&) = delete;
    //
    std::wstring static GetConfigPath();
    //COM3
    inline static std::wstring wsPortName;
    //port name long
    inline static std::wstring wsPortNameLong;
    //якщо порт знайшовся і з ним можна працювати
    inline static bool bPortAvaliable;


};

