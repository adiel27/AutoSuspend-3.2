#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <windows.h>

// Menulis log aktivitas ke file
void LogAction(const char* action, const char* processName, DWORD pid);

#endif // LOG_UTILS_H
