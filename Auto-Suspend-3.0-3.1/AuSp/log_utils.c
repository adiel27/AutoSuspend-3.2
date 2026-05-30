#include "log_utils.h"
#include <stdio.h>
#include <time.h>

#define LOG_FILE "SuspendLog.txt"

void LogAction(const char* action, const char* processName, DWORD pid) {
    FILE* file = fopen(LOG_FILE, "a");
    if (!file) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s [PID: %lu]\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        action, processName, pid);

    fclose(file);

     printf("[%04d-%02d-%02d %02d:%02d:%02d] %s: %s [PID: %lu]\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        action, processName, pid);
}
