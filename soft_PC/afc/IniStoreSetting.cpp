#include "IniStoreSetting.h"
//---------------------------------------------------------
//---------------------------------------------------------
void IniStoreSetting::SaveSelectedPort(const std::wstring& portName)
{
        std::wstring file = GetConfigPath();
        WritePrivateProfileStringW(
            L"Settings",
            L"SelectedPort",
            portName.c_str(),
            file.c_str());
        //
        wsPortName = portName;
        bPortAvaliable = true;
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyFrom(const std::wstring& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyFrom",
        value.c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyFrom(const std::string& value)
{
    //
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyFrom",
        std::wstring(value.begin(), value.end()).c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyTo(const std::wstring& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyTo",
        value.c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyTo(const std::string& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyTo",
        std::wstring(value.begin(), value.end()).c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyStep(const std::wstring& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyStep",
        value.c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveFrequencyStep(const std::string& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"FrequencyStep",
        std::wstring(value.begin(), value.end()).c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveOnlyVout(const std::wstring& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"OnlyVout",
        value.c_str(),
        file.c_str());
}
//---------------------------------------------------------
void IniStoreSetting::SaveOnlyVout(const std::string& value)
{
    std::wstring file = GetConfigPath();
    WritePrivateProfileStringW(
        L"Settings",
        L"OnlyVout",
        std::wstring(value.begin(), value.end()).c_str(),
        file.c_str());
}
//---------------------------------------------------------
std::wstring IniStoreSetting::LoadSelectedPort()
{
    wchar_t buffer[64] = { 0 };
    wsPortName.clear();
    std::wstring file = GetConfigPath();
    GetPrivateProfileStringW(
        L"Settings",
        L"SelectedPort",
        L"",
        buffer,
        64,
        file.c_str());

    wsPortName = buffer;
    return buffer;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::LoadFrequencyFrom()
{
    wchar_t buffer[64] = { 0 };
    std::wstring file = GetConfigPath();
    GetPrivateProfileStringW(
        L"Settings",
        L"FrequencyFrom",
        L"455000",
        buffer,
        64,
        file.c_str());
    if (std::wcslen(buffer) == 0) {
        return L"455000";
    }
    return buffer;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::LoadFrequencyTo()
{
    wchar_t buffer[64] = { 0 };
    std::wstring file = GetConfigPath();
    GetPrivateProfileStringW(
        L"Settings",
        L"FrequencyTo",
        L"475000",
        buffer,
        64,
        file.c_str());
    if (std::wcslen(buffer) == 0) {
        return L"475000";
    }
    return buffer;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::LoadFrequencyStep()
{
    wchar_t buffer[64] = { 0 };
    std::wstring file = GetConfigPath();
    GetPrivateProfileStringW(
        L"Settings",
        L"FrequencyStep",
        L"100",
        buffer,
        64,
        file.c_str());
    if (std::wcslen(buffer) == 0) {
        return L"100";
    }
    return buffer;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::LoadOnlyVout()
{
    wchar_t buffer[64] = { 0 };
    std::wstring file = GetConfigPath();
    GetPrivateProfileStringW(
        L"Settings",
        L"OnlyVout",
        L"1",
        buffer,
        64,
        file.c_str());
    if (std::wcslen(buffer) == 0) {
        return L"1";
    }
    return buffer;
}
//---------------------------------------------------------
void IniStoreSetting::RestoreSelectedPort()
{
    bPortAvaliable = false;
    wsPortName.clear();
    std::wstring saved = LoadSelectedPort();
    if (saved.empty())
        return;

    for (size_t i = 0; i < g_ComPorts.size(); ++i)
    {
        if (g_ComPorts[i].first == saved)
        {
            g_SelectedPort = (int)i;
            CheckMenuRadioItem(
                g_hComMenu,
                IDM_COM_BASE,
                IDM_COM_BASE + g_ComPorts.size() - 1,
                IDM_COM_BASE + i,
                MF_BYCOMMAND);
            //
            wsPortName = saved;
            wsPortNameLong = g_ComPorts[i].second;
            bPortAvaliable = true;
            break;
        }
    }
    return;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::GetPortName()
{
    return wsPortName;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::GetPortNameLong()
{
    return wsPortNameLong;
}
//---------------------------------------------------------
void IniStoreSetting::SetPortName(const std::wstring& value)
{
    wsPortName = value;
}
//---------------------------------------------------------
void IniStoreSetting::SetPortNameLong(const std::wstring& value)
{
    wsPortNameLong = value;
}
//---------------------------------------------------------
bool IniStoreSetting::GetPortAvaliable()
{
    return bPortAvaliable;
}
//---------------------------------------------------------
std::wstring IniStoreSetting::GetConfigPath()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring full = path;
    size_t pos = full.find_last_of(L"\\/");
    full = full.substr(0, pos + 1);
    full += L"config.ini";
    return full;
}
//---------------------------------------------------------
