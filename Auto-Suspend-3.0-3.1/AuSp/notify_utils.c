#include "notify_utils.h"
#include <windows.h>

void ShowStartupNotification() {
    MessageBox(NULL,
        "AutoSuspend telah aktif dan berjalan di latar belakang.",
        "AutoSuspend",
        MB_OK | MB_ICONINFORMATION);
}
