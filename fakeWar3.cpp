#include <windows.h>

#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>

static const char windowParam[] = " -window";
static char defaultConfig[] = R"("C:\Games\Warcraft III 1.26\realWar3.exe" -window)";

void fitWindowToMonitor(HWND hWnd) {
    RECT rc;
    HMONITOR hMonitor;
    MONITORINFO mi;

    rc = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

    GetWindowRect(hWnd, &rc);
    hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
    if (hMonitor) {
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfo(hMonitor, &mi))
            rc = mi.rcMonitor;
    }

    rc.right = rc.right - rc.left;
    rc.bottom = rc.bottom - rc.top;

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
    startupInfo.cb = sizeof(startupInfo);

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

std::filesystem::path getExecutablePath() {
    std::vector<wchar_t> path(MAX_PATH);
    int length = GetModuleFileNameW(NULL, path.data(), path.size());
    if (length >= path.size()) {
        path.resize(length);
    }

    GetModuleFileNameW(NULL, path.data(), path.size());
    return path.data();
}

int main(int argc, char *argv[]) {
    std::vector<char> config;
    std::filesystem::path configPath = getExecutablePath();

    configPath.remove_filename();
    configPath /= "fakeWar3.cfg";

    if (std::ifstream configFile(configPath); configFile.is_open()) {
        config = std::vector<char>(
            (std::istreambuf_iterator<char>(configFile)),
            std::istreambuf_iterator<char>()
        );

        config.insert(config.end(), std::begin(windowParam), std::end(windowParam) - 1);
    } else {
        std::cout << "Config file not found. Using default path for executable\n";
        config.insert(config.end(), std::begin(defaultConfig), std::end(defaultConfig) - 1);
    }

    for (int i = 1; i < argc; i++) {
        size_t length = strlen(argv[i]);
        config.push_back(' ');
        config.insert(config.end(), argv[i], argv[i] + length);
    }

    config.push_back('\0');
    std::cout << "Launching command :: ["<< config.data() << "]\n";
    return launchGame(config.data());
}
