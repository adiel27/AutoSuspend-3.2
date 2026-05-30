#include "window_utils.h"

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    DWORD windowPid;
    GetWindowThreadProcessId(hwnd, &windowPid);

    // Cocokkan PID dan pastikan jendela terlihat
    if ((DWORD)lParam == windowPid && IsWindowVisible(hwnd)) {
        return FALSE; // Ditemukan, hentikan enumerasi
    }
    return TRUE; // Lanjutkan enumerasi
}

BOOL HasVisibleWindow(DWORD pid) {
    return !EnumWindows(EnumWindowsCallback, (LPARAM)pid);
}

typedef struct {
    DWORD pid;
    HWND hwnd;
} handle_data;

// Fungsi callback untuk mencari jendela yang cocok dengan PID
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    handle_data* data = (handle_data*)lParam;
    DWORD process_id;
    GetWindowThreadProcessId(hwnd, &process_id);

    // Cari jendela yang utama, terlihat, dan punya judul
    if (data->pid != process_id || !IsWindowVisible(hwnd) || GetParent(hwnd) != NULL) {
        return TRUE;
    }
    data->hwnd = hwnd;
    return FALSE;
}

HWND GetMainWindowHandle(DWORD pid) {
    handle_data data = {pid, NULL};
    EnumWindows(EnumWindowsProc, (LPARAM)&data);
    return data.hwnd;
}
