#include "process_utils.h"
#include <stdio.h>
#include <windows.h>

int main() {
    // Memuat konfigurasi dari config.txt saat startup
    LoadConfig();

    // Loop monitoring utama
    while (1) {
        MonitorProcesses();
        Sleep(10000); // Interval 10 detik
    }
    return 0;
}
