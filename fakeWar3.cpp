#include <windows.h>

#include <iostream>
#include <vector>
#include <fstream>

#define MAX_COMMAND_SIZE 1024

// TODO: add multi-monitor support

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

    int cxScreen = GetSystemMetrics(SM_CXSCREEN);
    int cyScreen = GetSystemMetrics(SM_CYSCREEN);
    MoveWindow(hWnd, 0, 0, cxScreen, cyScreen, true);
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

        char parameter[] = "\" -window";
        config.insert(config.end(), std::begin(parameter), std::end(parameter));
        config.insert(config.begin(), '"');

        return launchGame(config.data());
    } else {
        char defaultPath[] = R"("C:\Games\Warcraft III 1.26\realWar3.exe" -window)";
        return launchGame(defaultPath);
    }
}
