#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <windows.h>

// --- Fungsi Inisialisasi ---

/**
 * Membaca config.txt untuk mengisi daftar Excluded dan Efficiency.
 * Dipanggil sekali di main() sebelum loop monitoring.
 */
void LoadConfig();

/**
 * Menambahkan nama proses ke daftar dinamis secara manual (jika diperlukan).
 */
void AddToDynamicList(char*** list, int* count, const char* name);
void AddToExcluded(const char* name);
void AddToEfficiency(const char* name);


// --- Fungsi Pengecekan ---

/**
 * Mengecek apakah proses ada di daftar pengecualian default atau user.
 */
BOOL IsExcluded(const char* processName);

/**
 * Mengecek apakah proses hanya boleh masuk mode efisiensi (tidak boleh suspend).
 */
BOOL IsEfficiencyOnly(const char* processName);


// --- Fungsi Kontrol Thread & Resource ---



/**
 * Membangunkan semua thread dalam proses sampai suspend count menjadi 0.
 */
void ResumeProcess(DWORD pid);


// --- Fungsi Inti Monitoring ---

/**
 * Fungsi utama yang melakukan scanning proses, pengecekan timer,
 * trimming RAM, dan manajemen status (Active/Efficiency/Suspend).
 */
void MonitorProcesses();

#endif // PROCESS_UTILS_H
