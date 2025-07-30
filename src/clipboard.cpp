#include <fstream>
#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>

void writeClipboardToFile(const std::wstring& filePath) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string filePathUtf8 = conv.to_bytes(filePath);

    std::ofstream file(filePathUtf8, std::ios::trunc);
    if (!file.is_open()) return;

    if (!OpenClipboard(nullptr)) return;

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        LPCWSTR pszText = static_cast<LPCWSTR>(GlobalLock(hData));
        if (pszText) {
            std::wstring wstr(pszText);
            std::string utf8str = conv.to_bytes(wstr);
            file << utf8str;
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
}
