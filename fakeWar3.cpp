#include <windows.h>

#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <locale>

struct res{
    int x;
    int y;

    bool operator== (const res& other) {
        return x == other.x && y == other.y;
    }
    bool operator!= (const res& other) {
        return !(*this == other);
    }
};

struct {
    std::string location;
    std::string arguments;
    bool makeBorderless;
    res resolution;
} config;

enum parse_state {
    identifier,
    value,
    string_value,
    skipping
};

enum identifiers {
    location,
    windowed,
    borderless,
    resolution,
    arguments,
    unknown
};

std::string toLower(const std::string& str) {
    static std::locale loc;
    std::string result = "";

    for (size_t i=0; i < str.length(); i++)
        result += std::tolower(str[i], loc);

    return result;
}

void fitWindowToMonitor(HWND hWnd) {
    RECT rc;
    HMONITOR hMonitor;
    MONITORINFO mi;

    rc = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

    GetWindowRect(hWnd, &rc);

    if (config.resolution == (res){-1, -1}) {
        hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
        if (hMonitor) {
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfo(hMonitor, &mi))
                rc = mi.rcMonitor;
        }

        rc.right = rc.right - rc.left;
        rc.bottom = rc.bottom - rc.top;
    }
    else {
        rc.right = config.resolution.x + rc.left;
        rc.bottom = config.resolution.y + rc.top;
    }

    MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, true);
}

void setBorderlessWindowStyle(HWND hWnd) {
    // set to borderless style and fullscreen
    LONG_PTR wndStyle = GetWindowLongPtrA(hWnd, GWL_STYLE);
    wndStyle &= ~(WS_CAPTION | WS_SIZEBOX);
    SetWindowLongPtrA(hWnd, GWL_STYLE, wndStyle);
}

void setUpGameWindow() {
    // wait for main window to be created
    HWND hWnd;
    do {
        Sleep(200);
        hWnd = FindWindowA("Warcraft III", "Warcraft III");
    } while (!hWnd);

    if (config.makeBorderless)
        setBorderlessWindowStyle(hWnd);

    if (config.makeBorderless || config.resolution != (res){-1, -1})
        fitWindowToMonitor(hWnd);
}

int launchGame() {
    STARTUPINFO startupInfo = { 0 };
    PROCESS_INFORMATION processInformation = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    std::string commandLine = config.location + ' ' + config.arguments;

    if(CreateProcess(NULL, commandLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation)) {
        setUpGameWindow();

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

identifiers str2identifier(const std::string& str) {
    if (str == "Location")
        return identifiers::location;
    if (str == "Windowed")
        return identifiers::windowed;
    if (str == "Borderless")
        return identifiers::borderless;
    if (str == "Resolution")
        return identifiers::resolution;
    if (str == "Arguments")
        return identifiers::arguments;

    return unknown;
}

void parseValue(const identifiers identifier, const std::string& value) {
    switch (identifier) {
        case identifiers::location:
            config.location = value;
            break;

        case identifiers::windowed:
            if (toLower(value) == "true")
                config.arguments += " -window ";
            break;

        case identifiers::borderless:
            config.makeBorderless = toLower(value) == "true";
            break;

        case identifiers::resolution: {
            if (toLower(value) == "auto") {
                config.resolution = {-1, -1};
                return;
            }

            size_t index = value.find('x');
            std::string number;
            if (index != std::string::npos) {
                try {
                    config.resolution.x = std::stoi(value.substr(0, index));
                    config.resolution.y = std::stoi(value.substr(index + 1));
                    return;
                }
                catch (...) { }
            }
            std::cout << "The format of the resultion has to be \"<number>x<number>\"\n";
        } break;

        case identifiers::arguments:
            config.arguments += value;
            break;

        default:
            std::cout << "Unknown identifier type\n";
    }
}

void parseConfig(const std::vector<char>& str) {
    config = {"", "", false, {-1, -1} };

    if (str.empty()) {
        config.location = "C:\\Games\\Warcraft III 1.26\\realWar3.exe";
        config.arguments = "-window";
        config.makeBorderless = true;
        return;
    }

    parse_state state = parse_state::identifier;
    identifiers currentIdentifer;

    std::string literal;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '\n') {
            parseValue(currentIdentifer, literal);
            state = parse_state::identifier;
            literal = "";
            continue;
        }

        switch (state) {
            case parse_state::identifier:
                if (str[i] == '=') {
                    state = parse_state::value;
                    currentIdentifer = str2identifier(literal);
                    literal = "";

                    continue;
                }
                if (!isspace(str[i]))
                    literal += str[i];
                break;

            case parse_state::value:
                if (!isspace(str[i]))
                    if (str[i] == '"') {
                        state = parse_state::string_value;
                        continue;
                    }
                    literal += str[i];
                break;

            case parse_state::string_value:
                if (str[i] == '"') {
                    state = parse_state::skipping;
                    continue;
                }
                literal += str[i];
                break;

            default:
                continue;
        }

    }
}

int main(int argc, char *argv[]) {
    std::vector<char> configStr;
    std::filesystem::path configPath = getExecutablePath();

    configPath.remove_filename();
    configPath /= "fakeWar3.cfg";

    if (std::ifstream configFile(configPath); configFile.is_open()) {
        configStr = std::vector<char>(
            (std::istreambuf_iterator<char>(configFile)),
            std::istreambuf_iterator<char>()
        );
    }

    parseConfig(configStr);

    for (int i = 1; i < argc; i++) {
        config.arguments += ' ';
        config.arguments += argv[i];
    }

    std::cout << "Launching command :: [" << config.location << " " << config.arguments << "]\n";
    return launchGame();
}
