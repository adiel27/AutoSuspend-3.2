#include "process_utils.h"
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <psapi.h>
#include "window_utils.h"
#include "cpu_utils.h"
#include "log_utils.h"

// --- DEFINE WINDOWS EFFICIENCY MODE API ---
#ifndef PROCESS_POWER_THROTTLING_CURRENT_VERSION
typedef struct _PROCESS_POWER_THROTTLING_STATE {
    ULONG Version;
    ULONG ControlMask;
    ULONG StateMask;
} PROCESS_POWER_THROTTLING_STATE, *PPROCESS_POWER_THROTTLING_STATE;

#define PROCESS_POWER_THROTTLING_CURRENT_VERSION 1
#define PROCESS_POWER_THROTTLING_EXECUTION_SPEED 0x1
#endif

// --- DEKLARASI TRACKER TIMER ---
typedef struct {
    DWORD pid;
    time_t idle_start_time;
} IdleTracker;

IdleTracker trackers[1024];
int tracker_count = 0;

// --- DAFTAR PROSES ---
const char* default_excluded[] = {
    "System", "svchost.exe", "explorer.exe", "wininit.exe", "csrss.exe",
    "services.exe", "lsass.exe", "igfx.exe", "amddvr.exe",
    "RadeonSoftware.exe", "DolbyDAX2.exe", "DolbyCPL.exe",
    "MsMpEng.exe", "AuSp.exe", "BhcMgr.exe"
};

char** dynamic_excluded = NULL;
int excluded_count = 0;
char** efficiency_list = NULL;
int efficiency_count = 0;

// --- FUNGSI MANAJEMEN CONFIG ---

void AddToDynamicList(char*** list, int* count, const char* name) {
    char** temp = realloc(*list, sizeof(char*) * (*count + 1));
    if (temp) {
        *list = temp;
        (*list)[*count] = _strdup(name);
        (*count)++;
    }
}

void LoadConfig() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    char *lastBackslash = strrchr(path, '\\');
    if (lastBackslash) {
        *(lastBackslash + 1) = '\0';
        strcat(path, "config.txt");
    }

    FILE *file = fopen(path, "r");
    if (!file) {
        LogAction("Config", "Failed to find config.txt at exe path", 0);
        return;
    }
    char line[100], section[20] = "";
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '[') {
            strcpy(section, line);
            continue;
        }
        if (strlen(line) > 1) {
            if (strcmp(section, "[EXCLUDED]") == 0) AddToDynamicList(&dynamic_excluded, &excluded_count, line);
            else if (strcmp(section, "[EFFICIENCY]") == 0) AddToDynamicList(&efficiency_list, &efficiency_count, line);
        }
    }
    fclose(file);
}

// --- FUNGSI PENGECEKAN ---

BOOL IsExcluded(const char* processName) {
    for (int i = 0; i < sizeof(default_excluded) / sizeof(default_excluded[0]); i++) {
        if (_stricmp(processName, default_excluded[i]) == 0) return TRUE;
    }
    for (int i = 0; i < excluded_count; i++) {
        if (_stricmp(processName, dynamic_excluded[i]) == 0) return TRUE;
    }
    return FALSE;
}

BOOL IsEfficiencyOnly(const char* processName) {
    for (int i = 0; i < efficiency_count; i++) {
        if (_stricmp(processName, efficiency_list[i]) == 0) return TRUE;
    }
    return FALSE;
}

// --- FUNGSI TIMER TRACKER ---

time_t GetOrSetIdleTime(DWORD pid) {
    for (int i = 0; i < tracker_count; i++) {
        if (trackers[i].pid == pid) return trackers[i].idle_start_time;
    }
    if (tracker_count < 1024) {
        trackers[tracker_count].pid = pid;
        trackers[tracker_count].idle_start_time = time(NULL);
        tracker_count++;
        return trackers[tracker_count - 1].idle_start_time;
    }
    return time(NULL);
}

void ResetIdleTracker(DWORD pid) {
    for (int i = 0; i < tracker_count; i++) {
        if (trackers[i].pid == pid) {
            trackers[i] = trackers[tracker_count - 1];
            tracker_count--;
            break;
        }
    }
}

// --- FUNGSI WINDOWS EFFICIENCY MODE CORE VIA KERNEL32 ---

void SetWindowsEfficiencyMode(HANDLE hProcess, BOOL enable) {
    PROCESS_POWER_THROTTLING_STATE powerThrottling;
    RtlZeroMemory(&powerThrottling, sizeof(powerThrottling));

    powerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    powerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    powerThrottling.StateMask = enable ? PROCESS_POWER_THROTTLING_EXECUTION_SPEED : 0;

    typedef BOOL (WINAPI *pSetProcessPowerThrottling)(HANDLE, PROCESS_POWER_THROTTLING_STATE*);
    pSetProcessPowerThrottling SetProcessPowerThrottling =
        (pSetProcessPowerThrottling)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetProcessPowerThrottling");

    if (SetProcessPowerThrottling) {
        SetProcessPowerThrottling(hProcess, &powerThrottling);
    }
}

// --- KONTROL THREAD ---

void ResumeProcess(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
                if (hThread) {
                    int resumeCount;
                    do {
                        resumeCount = ResumeThread(hThread);
                    } while (resumeCount > 0 && resumeCount != (DWORD)-1);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);
}

// --- MONITOR UTAMA ---

void MonitorProcesses() {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    const int GRACE_PERIOD = 300; // 5 Menit timer

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    HWND foregroundWnd = GetForegroundWindow();
    DWORD foregroundPid = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundPid);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (IsExcluded(pe32.szExeFile)) continue;
            if (!HasVisibleWindow(pe32.th32ProcessID)) continue;

            HWND hMainWnd = GetMainWindowHandle(pe32.th32ProcessID);
            BOOL isMinimized = IsIconic(hMainWnd);
            double cpu = GetCPUUsage(pe32.th32ProcessID);
            BOOL isIdle = cpu < 0.1;
            BOOL isNotForeground = (pe32.th32ProcessID != foregroundPid);

            // =================================================================
            // STRUKTUR KONDISI 3-ARAH (BOOSTER vs ECO THROTTLE vs BG ACTIVE)
            // =================================================================

            if (!isNotForeground) {
                // 1. JALUR KARPET MERAH: Aplikasi Foreground (System Booster)
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS); // Boost prioritas CPU
                    SetWindowsEfficiencyMode(hProcess, FALSE);       // Matikan pembatasan daya
                    CloseHandle(hProcess);
                }
                ResumeProcess(pe32.th32ProcessID);
                ResetIdleTracker(pe32.th32ProcessID);
                LogAction("BOOST: Active High Priority", pe32.szExeFile, pe32.th32ProcessID);

            } else if (isMinimized || isIdle) {
                // 2. JALUR PENGEKANGAN: Jendela di-minimize ATAU didiamkan hingga idle
                HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    // Selalu turunkan kasta CPU dan peras RAM secara berkala
                    SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS);
                    EmptyWorkingSet(hProcess);

                    // Cek Timer Transisi untuk peningkatan Eco Mode tingkat lanjut
                    time_t startTime = GetOrSetIdleTime(pe32.th32ProcessID);
                    double elapsed = difftime(time(NULL), startTime);

                    if (elapsed > GRACE_PERIOD) {
                        // Jika melewati batas waktu (Timer Out), kunci ke Windows Efficiency Mode resmi
                        SetWindowsEfficiencyMode(hProcess, TRUE);
                        LogAction("Eco Mode: Efficiency Mode (Timer Out)", pe32.szExeFile, pe32.th32ProcessID);
                    } else {
                        // Selama masa tenggang, biarkan berjalan di prioritas rendah standar
                        SetWindowsEfficiencyMode(hProcess, FALSE);
                        LogAction("Eco Mode: Low Priority (Grace Period)", pe32.szExeFile, pe32.th32ProcessID);
                    }
                    CloseHandle(hProcess);
                }
                ResumeProcess(pe32.th32ProcessID); // Jamin thread tetap sadar dan bernapas normal

            } else {
                // 3. JALUR BACKGROUND ACTIVE: Aplikasi background sibuk (misal: Render/Download)
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
                    SetWindowsEfficiencyMode(hProcess, FALSE); // Kembalikan performa penuh latar belakang
                    CloseHandle(hProcess);
                }
                ResumeProcess(pe32.th32ProcessID);
                ResetIdleTracker(pe32.th32ProcessID);
                LogAction("Background Active (Normal Priority)", pe32.szExeFile, pe32.th32ProcessID);
            }

        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
}
