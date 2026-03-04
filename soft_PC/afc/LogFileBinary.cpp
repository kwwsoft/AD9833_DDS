#include "LogFileBinary.h"

// --- Constructor to open or create a binary file for logging ---
LogFileBinary::LogFileBinary(const char* filename)
{
	file = fopen(filename, "ab"); 
	if (!file) {
		perror("Failed to open log file");
	}
}
// --- Destructor to ensure the file is closed when the object is destroyed ---
LogFileBinary::~LogFileBinary()
{
	closeFile();
}
// --- Method to write a log message to the file ---
//char*
void LogFileBinary::writeLogA(const char* logMessage)
{
	if (file) {
		fwrite(logMessage, sizeof(char), strlen(logMessage), file);
		fflush(file); // Ensure data is written to the file immediately
	}
}
//wchar_t
void LogFileBinary::writeLogW(const wchar_t* logMessage)
{
	if (file) {
		fwrite(logMessage, sizeof(wchar_t), wcslen(logMessage), file);
		fflush(file); // Ensure data is written to the file immediately
	}
}
// --- Method to write raw bytes to the log file ---
void LogFileBinary::writeLogBytes(const uint8_t* data, size_t length)
{
	if (file) {
		fwrite(data, sizeof(uint8_t), length, file);
		fflush(file); // Ensure data is written to the file immediately
	}
}
// --- Method to close the file ---
void LogFileBinary::closeFile()
{
	if (file) {
		fclose(file);
		file = nullptr;
	}
}
// --- End of LogFileBinary.cpp ---
// //--------------------------------------------------------------
