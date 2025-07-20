#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tchar.h>
#include <iphlpapi.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#include <winioctl.h>
#include <sysinfoapi.h>
#include <wbemidl.h>
#include <winhttp.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "winhttp.lib")

std::vector<std::pair<std::string, std::string>> getWMIInfo(const std::wstring &wmiClass, const std::vector<std::wstring> &fields)
{
    std::vector<std::pair<std::string, std::string>> result;

    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return result;

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    if (FAILED(hres) && hres != RPC_E_TOO_LATE)
    {
        CoUninitialize();
        return result;
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (void **)&pLoc);
    if (FAILED(hres))
    {
        CoUninitialize();
        return result;
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(
        BSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, 0, 0, &pSvc);
    if (FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    std::wstring query = L"SELECT * FROM " + wmiClass;
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        BSTR(L"WQL"),
        BSTR(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
            break;

        for (const auto &field : fields)
        {
            VARIANT vtProp;
            VariantInit(&vtProp);
            hr = pclsObj->Get(field.c_str(), 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
            {
                std::wstring ws(vtProp.bstrVal);
                std::string value(ws.begin(), ws.end());
                std::string key(field.begin(), field.end());
                result.push_back({key, value});
            }
            VariantClear(&vtProp);
        }
        pclsObj->Release();
    }

    if (pEnumerator)
        pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return result;
}
std::string getIpAdress()
{
    HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession)
    {
        std::cerr << "WinHttpOpen failed: " << GetLastError() << std::endl;
        return "Error";
    }

    HINTERNET hConnect = WinHttpConnect(hSession, L"ifconfig.me", INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect)
    {
        std::cerr << "WinHttpConnect failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return "Error";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/ip",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    if (!hRequest)
    {
        std::cerr << "WinHttpOpenRequest failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "Error";
    }

    std::string result;
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    {
        std::cerr << "WinHttpSendRequest failed: " << GetLastError() << std::endl;
    }
    else if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        std::cerr << "WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
    }
    else
    {
        DWORD dwSize = 0;
        do
        {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0)
                break;

            std::vector<char> buffer(dwSize + 1, 0);
            DWORD bytesRead = 0;
            if (WinHttpReadData(hRequest, buffer.data(), dwSize, &bytesRead) && bytesRead > 0)
            {
                result.append(buffer.data(), bytesRead);
            }
        } while (dwSize > 0);

        size_t start = result.find_first_not_of(" \r\n\t");
        size_t end = result.find_last_not_of(" \r\n\t");
        if (start != std::string::npos && end != std::string::npos)
            result = result.substr(start, end - start + 1);
        else
            result.clear();
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

void writeSystemInfoToFile(const std::wstring &filePath)
{
    std::wofstream out(filePath.c_str(), std::ios::app);
    if (!out.is_open())
        return;

    out << L"=== SYSTEM ===\n";
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    out << L"CPU Architecture: ";
    switch (siSysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        out << L"x64 (AMD or Intel)\n";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        out << L"ARM\n";
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        out << L"Intel Itanium-based\n";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        out << L"x86\n";
        break;
    default:
        out << L"Unknown architecture\n";
        break;
    }
    out << L"CORES: " << siSysInfo.dwNumberOfProcessors << L"\n";
    out << L"PAGE SIZE: " << siSysInfo.dwPageSize << L"\n";
    out << L"Minimum application address: " << siSysInfo.lpMinimumApplicationAddress << L"\n";
    out << L"Maximum application address: " << siSysInfo.lpMaximumApplicationAddress << L"\n";
    out << L"Processor mask: " << siSysInfo.dwActiveProcessorMask << L"\n";

    auto cpuInfo = getWMIInfo(L"Win32_Processor", {L"Name", L"Manufacturer", L"ProcessorId"});
    for (const auto &[key, value] : cpuInfo)
    {
        out << L"CPU " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto boardInfo = getWMIInfo(L"Win32_BaseBoard", {L"Product", L"Manufacturer", L"SerialNumber"});
    for (const auto &[key, value] : boardInfo)
    {
        out << L"Motherboard " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto biosInfo = getWMIInfo(L"Win32_BIOS", {L"Manufacturer", L"SMBIOSBIOSVersion", L"SerialNumber"});
    for (const auto &[key, value] : biosInfo)
    {
        out << L"BIOS " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto memInfo = getWMIInfo(L"Win32_PhysicalMemory", {L"Manufacturer", L"PartNumber", L"SerialNumber", L"Capacity"});
    for (const auto &[key, value] : memInfo)
    {
        out << L"RAM " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto gpuInfo = getWMIInfo(L"Win32_VideoController", {L"Name", L"AdapterRAM", L"DriverVersion"});
    for (const auto &[key, value] : gpuInfo)
    {
        out << L"GPU " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto diskInfo = getWMIInfo(L"Win32_DiskDrive", {L"Model", L"InterfaceType", L"SerialNumber", L"Size"});
    for (const auto &[key, value] : diskInfo)
    {
        out << L"DISK " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto netInfo = getWMIInfo(L"Win32_NetworkAdapter", {L"Name", L"MACAddress", L"Manufacturer", L"PNPDeviceID"});
    for (const auto &[key, value] : netInfo)
    {
        out << L"NET " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    auto monInfo = getWMIInfo(L"Win32_DesktopMonitor", {L"Name", L"PNPDeviceID", L"MonitorType"});
    for (const auto &[key, value] : monInfo)
    {
        out << L"MONITOR " << std::wstring(key.begin(), key.end()) << L": " << std::wstring(value.begin(), value.end()) << L"\n";
    }

    OSVERSIONINFOEXA osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    if (GetVersionExA((OSVERSIONINFOA*) &osvi))
    {
        out << L"\n=== WINDOWS VERSION ===\n";
        out << L"Major: " << osvi.dwMajorVersion << L"\n";
        out << L"Minor: " << osvi.dwMinorVersion << L"\n";
        out << L"Build: " << osvi.dwBuildNumber << L"\n";
        out << L"Platform: " << osvi.dwPlatformId << L"\n";
    }

    wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName) / sizeof(wchar_t);
    if (GetComputerNameW(computerName, &size))
        out << L"\nHost name: " << computerName << L"\n";
    wchar_t userName[256];
    DWORD userSize = sizeof(userName) / sizeof(wchar_t);
    if (GetUserNameW(userName, &userSize))
        out << L"Username: " << userName << L"\n";

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        out << L"\n=== MEM ===\n";
        out << L"Total phys: " << (statex.ullTotalPhys / (1024 * 1024)) << L" MB\n";
        out << L"Free phys: " << (statex.ullAvailPhys / (1024 * 1024)) << L" MB\n";
        out << L"Total virtual: " << (statex.ullTotalVirtual / (1024 * 1024)) << L" MB\n";
        out << L"Free virtual: " << (statex.ullAvailVirtual / (1024 * 1024)) << L" MB\n";
    }

    out << L"\n=== DISKS ===\n";
    DWORD drives = GetLogicalDrives();
    for (wchar_t letter = L'A'; letter <= L'Z'; ++letter)
    {
        if (drives & (1 << (letter - L'A')))
        {
            std::wstring root = std::wstring(1, letter) + L":\\";
            UINT type = GetDriveTypeW(root.c_str());
            out << L"DISK " << root << L" - ";
            switch (type)
            {
            case DRIVE_FIXED:
                out << L"HDD";
                break;
            case DRIVE_REMOVABLE:
                out << L"REMOVABLE";
                break;
            case DRIVE_CDROM:
                out << L"CD-ROM";
                break;
            case DRIVE_REMOTE:
                out << L"REMOVE";
                break;
            default:
                out << L"OTHER";
                break;
            }
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            if (GetDiskFreeSpaceExW(root.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes))
            {
                out << L" | TOTAL: " << (totalNumberOfBytes.QuadPart / (1024 * 1024)) << L" MB";
                out << L" | FREE: " << (totalNumberOfFreeBytes.QuadPart / (1024 * 1024)) << L" MB";
            }
            out << L"\n";
        }
    }

    out << L"\n=== NETWORK ===\n";
    ULONG bufLen = 0;
    GetAdaptersInfo(NULL, &bufLen);
    std::vector<BYTE> buffer(bufLen);
    IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO *)(buffer.data());
    if (GetAdaptersInfo(pAdapterInfo, &bufLen) == ERROR_SUCCESS)
    {
        while (pAdapterInfo)
        {
            out << L"NAME: " << std::wstring(pAdapterInfo->AdapterName, pAdapterInfo->AdapterName + strlen(pAdapterInfo->AdapterName)) << L"\n";
            out << L"DESC: " << std::wstring(pAdapterInfo->Description, pAdapterInfo->Description + strlen(pAdapterInfo->Description)) << L"\n";
            out << L"MAC: ";
            for (UINT i = 0; i < pAdapterInfo->AddressLength; i++)
                out << std::hex << (int)pAdapterInfo->Address[i] << (i + 1 < pAdapterInfo->AddressLength ? L"-" : L"");
            out << std::dec << L"\n";
            std::string publicIp = getIpAdress();
            out << L"IP (public): " << std::wstring(publicIp.begin(), publicIp.end()) << L"\n";
            out << L"IP (local): " << std::wstring(pAdapterInfo->IpAddressList.IpAddress.String, pAdapterInfo->IpAddressList.IpAddress.String + strlen(pAdapterInfo->IpAddressList.IpAddress.String)) << L"\n";
            out << L"Gateway: " << std::wstring(pAdapterInfo->GatewayList.IpAddress.String, pAdapterInfo->GatewayList.IpAddress.String + strlen(pAdapterInfo->GatewayList.IpAddress.String)) << L"\n";
            out << L"\n";
            pAdapterInfo = pAdapterInfo->Next;
        }
    }

    out << L"=== DEVICES ===\n";
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(NULL, L"USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA DeviceInfoData;
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); ++i)
        {
            WCHAR desc[1024];
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)desc, sizeof(desc), NULL))
            {
                out << L"DEVICE: " << desc;
                WCHAR loc[256];
                if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &DeviceInfoData, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)loc, sizeof(loc), NULL))
                {
                    out << L" (" << loc << L")";
                }
                out << L"\n";
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    out << L"\n=== UPTIME ===\n";
    DWORD uptime = GetTickCount64() / 1000;
    DWORD days = uptime / 86400;
    DWORD hours = (uptime % 86400) / 3600;
    DWORD mins = (uptime % 3600) / 60;
    DWORD secs = uptime % 60;
    out << L"UPTIME: " << days << L" days. " << hours << L" hours. " << mins << L" mins. " << secs << L" secs.\n";

    out << L"\n=== END ===\n\n";
    out.close();
}
