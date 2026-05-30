#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include <windows.h>

// Mengecek apakah proses dengan PID tertentu memiliki jendela GUI yang terlihat
BOOL HasVisibleWindow(DWORD pid);

HWND GetMainWindowHandle(DWORD pid);
BOOL HasVisibleWindow(DWORD pid);

#endif // WINDOW_UTILS_H
