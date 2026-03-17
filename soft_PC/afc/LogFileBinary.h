#pragma once

#define _CRT_SECURE_NO_DEPRECATE

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <Windows.h>
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
class LogFileBinary
{
	//відкрити або створити бінарний файл для запису логів
public:
	LogFileBinary(const char* filename);
	~LogFileBinary();
	//записати лог у файл
	void writeLogA(const char* logMessage);
	void writeLogW(const wchar_t* logMessage);
	void writeLog(const double logMessage);
	//записати в лог байти
	//записати в лог байти
	void writeLogBytes(const uint8_t* data, size_t length);
	//закрити файл
	void closeFile();
private:
	FILE* file;
	//--------------------------------------------------------------------------
};
/*
	LogFileBinary  logfile("afc_log.bin");

	for (const auto& p :ports) {
		std::wstring s1 = p.first.c_str();
		s1.append(L" --> ");
		s1.append(p.second.c_str());
		s1.append(L"\r\n");
		logfile.writeLogA((WideToANSI(s1.c_str()).c_str()));
	}
	logfile.closeFile();
*/

