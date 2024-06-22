/*

most of this repository was coded by me, but the logic of the program is copypasted from some old forums and websites of cpp

credits: zjuvee / tosted

*/

// HWID-grabber.cpp
#include <Windows.h>
#include <Lmcons.h>
#include <sddl.h>
#include <iostream>
#include <objbase.h>
#include <wbemidl.h>
#include <comdef.h>
#include <chrono>
#include <string>
#include <thread>
#include <iphlpapi.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace std;

void banner() {
    std::string logo = R"(
                      .----.
          .---------. | == |
          |.-"""""-.| |----|  
          ||       || | == | 
          ||       || |----| 
          |'-.....-'| |::::|
          `"")---(""` |___.|
         /:::::::::::\" _  "    coded by tosted uwu
        /:::=======:::\`\`\ 
        `"""""""""""""`  '-'   )";
    

    for (char c : logo) {
        cout << c;
        this_thread::sleep_for(chrono::nanoseconds(1));
    }
    std::cout << "\n" << std::endl;
}

// get HWID (hardware unique identifier)
void getHWID() {
    // get computer username
    WCHAR username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (!GetUserNameW(username, &username_len)) {
        return; // exit if cannot get the username
    }

    //============ get the HWID (SID) ============
    DWORD sidSize = 0;
    DWORD domainSize = 0;
    SID_NAME_USE sidType;
    LookupAccountNameW(NULL, username, NULL, &sidSize, NULL, &domainSize, &sidType);

    // check if buffer sizes are insufficient
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return; // exit if not an insufficient buffer error
    }

    // allocate memory for SID and domain
    PSID sid = (PSID)malloc(sidSize);
    WCHAR* domain = (WCHAR*)malloc(domainSize * sizeof(WCHAR));

    // lookup account name to get the SID
    if (!LookupAccountNameW(NULL, username, sid, &sidSize, domain, &domainSize, &sidType)) {
        free(sid);
        free(domain);
        return; // exit if cannot lookup account name
    }

    //===========================================

    // convert SID to string
    LPWSTR sidString;
    if (!ConvertSidToStringSidW(sid, &sidString)) {
        free(sid);
        free(domain);
        return; // exit if cannot convert SID to string
    }

    //std::wcout << L"HWID: " << sidString << std::endl;
    std::wstring hwidMessage = L"HWID: " + std::wstring(sidString) + L"\n";

    for (char c : hwidMessage) {
        cout << c;
        this_thread::sleep_for(chrono::nanoseconds(1));
    }

    // clean up allocated memory
    free(sid);
    free(domain);
    LocalFree(sidString);
}

// get UUID (universal unique identifier)
void getUUID() {

    HRESULT hResult; // variable to store result of COM operations
    IWbemLocator* pLocator = NULL; // pointer to IWbemLocator object
    IWbemServices* pServices = NULL; // pointer to IWbemServices object
    IEnumWbemClassObject* pEnumerator = NULL; // pointer to enumerator for WMI objects
    IWbemClassObject* pClassObject = NULL; // pointer to WMI class object
    ULONG uReturn = 0; // variable to store number of objects returned

    // initialize COM library for multithreaded use
    hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hResult)) {
        return;
    }

    // set security levels for the COM process
    hResult = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hResult)) {
        CoUninitialize();
        return;
    }

    // create IWbemLocator object to connect to WMI
    hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
    if (FAILED(hResult)) {
        CoUninitialize();
        return;
    }

    // connect to the ROOT\CIMV2 namespace
    hResult = pLocator->ConnectServer(BSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pServices);
    if (FAILED(hResult)) {
        pLocator->Release();
        CoUninitialize();
        return;
    }

    // set the authentication and impersonation levels for WMI
    hResult = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hResult)) {
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        return;
    }

    // execute WMI query to get UUID
    hResult = pServices->ExecQuery(BSTR(L"WQL"), BSTR(L"SELECT UUID FROM Win32_ComputerSystemProduct"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hResult)) {
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        return;
    }

    // get the first object in the enumerator
    hResult = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
    if (uReturn == 0) {
        pEnumerator->Release();
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        return;
    }

    VARIANT vtProp; // variable to store the UUID property uwu
    hResult = pClassObject->Get(L"UUID", 0, &vtProp, 0, 0); // get uuid
    if (SUCCEEDED(hResult)) {
        std::wstring uuidMessage = L"UUID: " + std::wstring(vtProp.bstrVal) + L"\n";

        for (char c : uuidMessage) {
            cout << c;
            this_thread::sleep_for(chrono::nanoseconds(1));
        }

        VariantClear(&vtProp); 
    }

    // release resources and uninitialize COM
    pClassObject->Release();
    pEnumerator->Release();
    pServices->Release();
    pLocator->Release();
    CoUninitialize();
}


// get username
void getUsername() {
    WCHAR username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    if (!GetUserNameW(username, &username_len)) {
        return; // exit if cannot get the username
    }

    std::wstring usernameMessage = L"Username: " + std::wstring(username) + L"\n";

    for (char c : usernameMessage) {
        cout << c;
        this_thread::sleep_for(chrono::nanoseconds(1));
    }
}

// get mac (copy pasted func)
void getMAC() {
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(AdapterInfo);
    AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));

    // allocate memory for adapterinfo; if allocation fails, exit
    if (AdapterInfo == NULL) {
        return;
    }

    // retrieve adapter information; handle buffer overflow if needed; if retrieval fails, exit
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);

        // exit if re-allocation fails
        if (AdapterInfo == NULL) {
            return;
        }
    }

    // getadaptersinfo succeeded; iterate through adapters to find MAC
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        while (pAdapterInfo) {
            if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET && pAdapterInfo->DhcpEnabled) {
                std::string macAddress = "";
                for (UINT i = 0; i < pAdapterInfo->AddressLength; i++) {
                    char buffer[3];
                    sprintf_s(buffer, sizeof(buffer), "%02X", pAdapterInfo->Address[i]);
                    macAddress += buffer;
                    if (i < pAdapterInfo->AddressLength - 1) {
                        macAddress += "-";
                    }
                }
                std::string macMessage = "MAC Address: " + macAddress + "\n";

                for (char c : macMessage) {
                    cout << c;
                    this_thread::sleep_for(chrono::nanoseconds(1));
                }

                break;
            }

            pAdapterInfo = pAdapterInfo->Next;
        }
    }

    if (AdapterInfo) {
        free(AdapterInfo);
    }
}

// get hdd serial number
void getHDD() {
    HRESULT hResult;
    IWbemLocator* pLocator = NULL;
    IWbemServices* pServices = NULL;
    IEnumWbemClassObject* pEnumerator = NULL;
    IWbemClassObject* pClassObject = NULL;
    ULONG uReturn = 0;

    // initialize COM library for multithreaded use
    hResult = CoInitializeEx(0, COINIT_MULTITHREADED);

    // set security levels
    hResult = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    // create IWbemLocator object to connect to WMI
    hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);

    // connect to the ROOT\CIMV2 namespace
    hResult = pLocator->ConnectServer(BSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pServices);

    // set the authentication and impersonation levels for WMI
    hResult = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    // execute WMI query to get HDD serial number
    hResult = pServices->ExecQuery(BSTR(L"WQL"), BSTR(L"SELECT SerialNumber FROM Win32_PhysicalMedia"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    // get the first object in the enumerator
    hResult = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);

    VARIANT vtProp;
    // retrieve HDD serial number from the WMI object
    hResult = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
    if (SUCCEEDED(hResult)) {
        std::wstring hddSerialMessage = L"HDD Serial Number: " + std::wstring(vtProp.bstrVal) + L"\n";

        for (char c : hddSerialMessage) {
            cout << c;
            this_thread::sleep_for(chrono::nanoseconds(1));
        }

        VariantClear(&vtProp); // clear variant after use uwu
    }

    // release
    pClassObject->Release();
    pEnumerator->Release();
    pServices->Release();
    pLocator->Release();
    CoUninitialize();
}


int main() {

    // set console properties
    SetConsoleTitleA("Unique Identifier Grabber");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    HWND hWnd = GetConsoleWindow();
    SetWindowPos(hWnd, NULL, 0, 0, 590, 420, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
    
    banner();
    
    // script kiddie moment
    string pir = "========================== Information ===========================\n";
    string pir2 = "==================================================================\n";

    for (char c : pir) {
        cout << c;
        this_thread::sleep_for(chrono::nanoseconds(1));
    }

    // get unique identifiers
    getUsername();
    getMAC();
    getHWID();
    getUUID();
    getHDD();

    for (char c : pir2) {
        cout << c;
        this_thread::sleep_for(chrono::nanoseconds(1));
    }

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
    std::cout << "\nPress any key to exit <3\n" << std::endl;
    cin.get();

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    return 0; 
}
