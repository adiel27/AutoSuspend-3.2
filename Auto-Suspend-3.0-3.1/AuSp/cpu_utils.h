#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include <windows.h>

// Mengembalikan estimasi penggunaan CPU oleh proses (dalam persen atau delta waktu)
double GetCPUUsage(DWORD pid);

#endif // CPU_UTILS_H
