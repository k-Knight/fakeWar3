#include <windows.h>

#include <vector>
#include <fstream>
#include <algorithm>

#define MAX_COMMAND_SIZE 1024

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"

void fitWindowToMonitor(HWND hWnd) {
    RECT rc;
    HMONITOR hMonitor;
    MONITORINFO mi;

    GetWindowRect(hWnd, &rc);
    hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
    if (hMonitor) {
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);
        rc = mi.rcMonitor;
    }
    else {
        rc = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    }

    rc.right = std::abs(rc.right - rc.left);
    rc.bottom = std::abs(rc.bottom - rc.top);

    MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, true);
}

void setBorderlessWindowStyle() {
    // wait for main window to be created
    HWND hWnd;
    do {
        Sleep(200);
        hWnd = FindWindowA("Warcraft III", "Warcraft III");
    } while (!hWnd);

    // set to borderless style and fullscreen
    LONG_PTR wndStyle = GetWindowLongPtrA(hWnd, GWL_STYLE);
    wndStyle &= ~(WS_CAPTION | WS_SIZEBOX);
    SetWindowLongPtrA(hWnd, GWL_STYLE, wndStyle);

    fitWindowToMonitor(hWnd);
}

int launchGame(char *commandLine) {
    STARTUPINFO startupInfo = { 0 };
    PROCESS_INFORMATION processInformation = { 0 };
    startupInfo.cb = sizeof( startupInfo );

    if(CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation)) {
        setBorderlessWindowStyle();

        // GameRanger binds itself to the process, therefore we must not exit until the game exits
        WaitForSingleObject(processInformation.hProcess, INFINITE);
        CloseHandle(processInformation.hProcess);
        CloseHandle(processInformation.hThread);

        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int main() {
    if (std::ifstream configFile("fakeWar3.cfg"); configFile.is_open()) {
        std::vector<char> config(
            (std::istreambuf_iterator<char>(configFile)),
            std::istreambuf_iterator<char>()
        );

        char parameter[] = " -window";
        config.insert(config.end(), std::begin(parameter), std::end(parameter));

        return launchGame(config.data());
    } else {
        char defaultPath[] = R"("C:\Games\Warcraft III 1.26\realWar3.exe" -window)";
        return launchGame(defaultPath);
    }
}
