#include "Window.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    FILE* FileProtector;
    if (FileProtector = fopen("C:\\Windows\\System32\\FileProtector.exe", "r")) {
        FILE* file;
        if (file = fopen("C:\\Windows\\System32\\drivers\\FileProtector.sys", "r")) {
            if (!IsDriverRunning(L"FileProtector")) {
                //MessageBoxA(NULL, "Driver Not Running", "", MB_OK | MB_ICONINFORMATION);
                StartDriverStartup(driverName);
                ExitProcess(1);
            }
        }
    }
    InitWindow(hInstance, nCmdShow);
    return 0;
}