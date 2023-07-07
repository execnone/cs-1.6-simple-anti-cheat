#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <tlhelp32.h>
#include <string>
#include <thread>

HANDLE GetProcessHandle(const std::string& processName) {
    HANDLE hProcess = NULL;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot. Error code: " << GetLastError() << std::endl;
        return NULL;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
    }

    return hProcess;
}

DWORD_PTR GetModuleBaseAddress(HANDLE hProcess, const std::string& moduleName) {
    HMODULE hModules[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        int moduleCount = cbNeeded / sizeof(HMODULE);

        for (int i = 0; i < moduleCount; ++i) {
            char modulePath[MAX_PATH];

            if (GetModuleFileNameExA(hProcess, hModules[i], modulePath, sizeof(modulePath))) {
                std::string fullPath(modulePath);
                size_t found = fullPath.find_last_of("\\");
                std::string modName = fullPath.substr(found + 1);

                if (modName == moduleName) {
                    return reinterpret_cast<DWORD_PTR>(hModules[i]);
                }
            }
        }
    }

    return 0;
}

void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { x, y };
    SetConsoleCursorPosition(hConsole, position);
}

bool acActive = false;

void ConsoleControlFunc()
{
    int x = 11;
    int y = 0;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Imleci gizle
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    SetConsoleTextAttribute(hConsole, 7);
    std::cout << "Anti-Cheat ";

    while (true)
    {
        if (acActive)
        {
            SetConsoleTextAttribute(hConsole, 10);
            setCursorPosition(x, y);
            std::cout << "Active "; // Windows
        }
        else
        {
            SetConsoleTextAttribute(hConsole, 12);
            setCursorPosition(x, y);
            std::cout << "Passive"; // Windows
        }
        std::cout << std::endl;
    }    

    return;
}

int main() {

    HANDLE hProcess = GetProcessHandle("hl.exe");

    if (!hProcess)
        return 1;

    std::thread consoleControlThread(ConsoleControlFunc);

    DWORD_PTR hwBase = GetModuleBaseAddress(hProcess, "hw.dll");
    DWORD_PTR m_dwWeaponId = (hwBase + 0x108DD90);
    DWORD dwWeaponId = 0;

    while (true)
    {
        if (hProcess)
        {
            if (!ReadProcessMemory(hProcess, (PBYTE*)m_dwWeaponId, &dwWeaponId, sizeof(dwWeaponId), 0))
                MessageBox(NULL, "read error", "error", MB_OK);

            if (dwWeaponId != 0)
            {
                acActive = true;
                HMODULE hModules[1024];
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
                    int moduleCount = cbNeeded / sizeof(HMODULE);

                    for (int i = 0; i < moduleCount; ++i) {
                        char modulePath[MAX_PATH];

                        if (GetModuleFileNameExA(hProcess, hModules[i], modulePath, sizeof(modulePath)))
                        {
                            //std::cout << "[" << i << "] Module Path : " << modulePath << std::endl;
                        }

                        if (i > 149)
                        {
                            MessageBox(NULL, "detected cheat!", "m0rphine anti-cheat", MB_OK);
                        }
                    }
                }
            }
            else
                acActive = false;
        }
    }

    CloseHandle(hProcess);

    return 0;
}
