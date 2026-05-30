#include "cpu_utils.h"
#include <psapi.h>
#include <stdio.h>

double GetCPUUsage(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return -1.0;

    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        CloseHandle(hProcess);
        return -1.0;
    }

    // Konversi FILETIME ke 64-bit integer
    ULARGE_INTEGER k, u;
    k.LowPart = ftKernel.dwLowDateTime;
    k.HighPart = ftKernel.dwHighDateTime;
    u.LowPart = ftUser.dwLowDateTime;
    u.HighPart = ftUser.dwHighDateTime;

    // Total waktu CPU dalam 100-nanosecond unit
    double totalTime = (double)(k.QuadPart + u.QuadPart) / 10000000.0; // dalam detik

    CloseHandle(hProcess);
    return totalTime;
}
