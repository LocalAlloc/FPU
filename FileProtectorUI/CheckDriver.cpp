#include "CheckDriver.h"

//// Function to check if a driver is running with the specified name
BOOL IsDriverRunning(const wchar_t* driverName)
{
    SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == NULL) {
        wprintf(L"Failed to open service control manager.\n");
        return FALSE;
    }

    BOOL isRunning = FALSE;
    DWORD bytesNeeded, servicesReturned, resumeHandle = 0;
    EnumServicesStatusExW(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle, NULL);

    if (GetLastError() != ERROR_MORE_DATA) {
        wprintf(L"Failed to retrieve required buffer size for services.\n");
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    BYTE* buffer = (BYTE*)malloc(bytesNeeded);
    if (buffer == NULL) {
        wprintf(L"Failed to allocate memory for buffer.\n");
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    ENUM_SERVICE_STATUS_PROCESSW* services = (ENUM_SERVICE_STATUS_PROCESSW*)buffer;
    if (!EnumServicesStatusExW(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL, (LPBYTE)services, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle, NULL)) {
        wprintf(L"Failed to retrieve services information.\n");
        free(buffer);
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    for (DWORD i = 0; i < servicesReturned; i++) {
        if (wcscmp(services[i].lpServiceName, driverName) == 0) {
            SC_HANDLE hService = OpenServiceW(hSCManager, services[i].lpServiceName, SERVICE_QUERY_STATUS);
            if (hService != NULL) {
                SERVICE_STATUS_PROCESS serviceStatus;
                DWORD bytesNeeded;
                if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
                    if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
                        isRunning = TRUE;
                        break;
                    }
                }
                CloseServiceHandle(hService);
            }
        }
    }

    free(buffer);
    CloseServiceHandle(hSCManager);

    return isRunning;
}
void AddExclusion() {
    std::wstring directoryPath = L"C:\\Windows\\System32"; // Specify the directory path you want to exclude

    // Build the PowerShell command to add the exclusion
    std::wstring command = L"Add-MpPreference -ExclusionPath \"" + directoryPath + L"\"";

    // Set up the process information
    STARTUPINFO startupInfo = {};
    PROCESS_INFORMATION processInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE; // Hide the PowerShell window

    // Execute the PowerShell command
    std::wstring powerShellCommand = L"powershell.exe -Command \"" + command + L"\"";
    BOOL result = CreateProcess(NULL, const_cast<LPWSTR>(powerShellCommand.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo);

    if (result)
    {
        //std::wcout << L"Windows Defender exclusion added successfully." << std::endl;

        // Wait for the PowerShell process to exit
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
    else
    {
        std::wcout << L"Failed to add Windows Defender exclusion." << std::endl;
        ExitProcess(1);
    }
}
void RemoveExclusion() {
    std::wstring directoryPath = L"C:\\Windows\\System32"; // Specify the directory path you want to remove the exclusion for

    // Build the PowerShell command to remove the exclusion
    std::wstring command = L"Remove-MpPreference -ExclusionPath \"" + directoryPath + L"\"";

    // Set up the process information
    STARTUPINFO startupInfo = {};
    PROCESS_INFORMATION processInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE; // Hide the PowerShell window

    // Execute the PowerShell command
    std::wstring powerShellCommand = L"powershell.exe -Command \"" + command + L"\"";
    BOOL result = CreateProcess(NULL, const_cast<LPWSTR>(powerShellCommand.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo);

    if (result)
    {
        //std::wcout << L"Windows Defender exclusion removed successfully." << std::endl;

        // Wait for the PowerShell process to exit
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
    else
    {
        //std::wcout << L"Failed to remove Windows Defender exclusion." << std::endl;
        ExitProcess(1);
    }
}
void ExeecuteKdu() {
    // Command to execute
    std::wstring command = L"cmd /c C:\\Windows\\System32\\kdu.exe -dse 0";

    // Set up the process information
    STARTUPINFO startupInfo = {};
    PROCESS_INFORMATION processInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE; // Hide the CMD window

    // Create the process
    BOOL result = CreateProcess(NULL, const_cast<LPWSTR>(command.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo);

    if (result)
    {
        //std::cout << "Command executed successfully." << std::endl;

        // Wait for the process to exit
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
    else
    {
        //std::cout << "Failed to execute the command." << std::endl;
        ExitProcess(1);
    }
}
// Function to stop a driver with the specified name
bool StopDriver(const std::wstring& driverName)
{
    if (!IsDriverRunning(L"FileProtector")) {
        MessageBoxA(NULL, "Dude Stop Pressing Me Please It Hurts!", "ERROR", MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service Control Manager", NULL, MB_OK | MB_ICONSTOP);
#ifdef _DEBUG
        std::cerr << "Failed to open the Service Control Manager: " << GetLastError() << std::endl;
#endif
        return false;
    }

    SC_HANDLE hService = OpenService(hSCManager, driverName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (hService == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service", NULL, MB_OK | MB_ICONSTOP);
#ifdef _DEBUG
        std::cerr << "Failed to open the service: " << GetLastError() << std::endl;
#endif
        CloseServiceHandle(hSCManager);
        return false;
    }

    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        MessageBoxA(NULL, "Failed To Stop The Service", NULL, MB_OK | MB_ICONSTOP);
#ifdef _DEBUG
        std::cerr << "Failed to stop the service: " << GetLastError() << std::endl;
#endif
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    MessageBoxA(NULL, "The Driver Was Stopped Successfully", "Success!", MB_OK | MB_ICONINFORMATION);

    return true;
}
bool StartDriver(const std::wstring& driverName)
{
    //if (!IsDriverRunning(L"FileProtector")) {
    //    MessageBoxA(NULL, "Dude Stop Pressing Me Please It Hurts!", "ERROR", MB_OK | MB_ICONSTOP);
    //    ExitProcess(1);
    //}
    MessageBoxA(NULL, "The Program Might Get Stuck For A While So Please Have Patience", "Patience", MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
    AddExclusion();
    ExtractResourceExe(IDR_EXE1, L"C:\\Windows\\System32\\kdu.exe");
    ExtractResourceDll(IDR_DLL1, L"C:\\Windows\\System32\\drv64.dll");
    ExeecuteKdu();
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service Control Manager", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to open the Service Control Manager: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE hService = OpenService(hSCManager, driverName.c_str(), SERVICE_START);
    if (hService == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to open the service: " << GetLastError() << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    if (!StartService(hService, 0, NULL))
    {
        MessageBoxA(NULL, "Failed To Start The Service", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to start the service: " << GetLastError() << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    DeleteFileA("C:\\Windows\\System32\\kdu.exe");
    DeleteFileA("C:\\Windows\\System32\\drv64.dll");
    RemoveExclusion();
    MessageBoxA(NULL, "Service Started Successfully!", "Success", MB_OK | MB_ICONINFORMATION);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return true;
}
bool StartDriverStartup(const std::wstring& driverName)
{
    //if (!IsDriverRunning(L"FileProtector")) {
    //    MessageBoxA(NULL, "Dude Stop Pressing Me Please It Hurts!", "ERROR", MB_OK | MB_ICONSTOP);
    //    ExitProcess(1);
    //}
    //MessageBoxA(NULL, "The Program Might Get Stuck For A While So Please Have Patience", "Patience", MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
    AddExclusion();
    ExtractResourceExe(IDR_EXE1, L"C:\\Windows\\System32\\kdu.exe");
    ExtractResourceDll(IDR_DLL1, L"C:\\Windows\\System32\\drv64.dll");
    ExeecuteKdu();
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service Control Manager", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to open the Service Control Manager: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE hService = OpenService(hSCManager, driverName.c_str(), SERVICE_START);
    if (hService == NULL)
    {
        MessageBoxA(NULL, "Failed To Open The Service", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to open the service: " << GetLastError() << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    if (!StartService(hService, 0, NULL))
    {
        MessageBoxA(NULL, "Failed To Start The Service", NULL, MB_OK | MB_ICONSTOP);
        //std::cerr << "Failed to start the service: " << GetLastError() << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    DeleteFileA("C:\\Windows\\System32\\kdu.exe");
    DeleteFileA("C:\\Windows\\System32\\drv64.dll");
    RemoveExclusion();
    //MessageBoxA(NULL, "Service Started Successfully!", "Success", MB_OK | MB_ICONINFORMATION);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return true;
}
void InstallDriver() {
    if (!ExtractResourceSys(IDR_SYS1, L"C:\\Windows\\System32\\FileProtector.sys")) {
        MessageBoxA(NULL, "Failed To Extract Driver!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }
    if (!ExtractResourceInf(IDR_INF1, L"C:\\Windows\\System32\\FileProtectorInstaller.inf")) {
        MessageBoxA(NULL, "Failed To Extract Driver Installer!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }
    ShellExecuteA(NULL, "open", "cmd", "/c rundll32.exe syssetup.dll,SetupInfObjectInstallAction DefaultInstall 128 C:\\Windows\\System32\\FileProtectorInstaller.inf", NULL, SW_HIDE);
    Sleep(5000);
    DeleteFileA("C:\\Windows\\System32\\FileProtectorInstaller.inf");
    DeleteFileA("C:\\Windows\\System32\\FileProtector.sys");
}
void UninstallDriver(const std::wstring& driverName) {
    StopDriver(driverName);
    system("sc delete FileProtector");
    removeTask();
    if (!DeleteFileA("C:\\Windows\\System32\\drivers\\FileProtector.sys")) {
        MessageBoxA(NULL, "Failed To Delete Driver File!\n\nYou Can Do It Manually Navigate to C:\\Windows\\System32\\drivers\\ \n\nAnd Then Delete \"FileProtector.sys\"", NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST);
    }
    if (!DeleteFileA("C:\\Windows\\System32\\FileProtector.exe")) {
        MessageBoxA(NULL, "Failed To Delete Driver Helper!\n\nYou Can Do It Manually Navigate to C:\\Windows\\System32\\ \n\nAnd Then Delete \"FileProtector.exe\"", NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST);
    }
}
bool ExtractResourceExe(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"EXE");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
bool ExtractResourceSys(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"SYS");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
bool ExtractResourceDll(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"DLL");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
bool ExtractResourceInf(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"INF");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
int setTask() {
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        ExitProcess(-1);
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        ExitProcess(1);
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc(Maruf)"));
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IActionCollection* pActionCollection = NULL;
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    if (FAILED(hr)) {
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IExecAction* pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
    if (FAILED(hr)) {
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\System32\\FileProtector.exe"));

    // Set the task to run on user logon
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ILogonTrigger* pLogonTrigger = NULL;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
    if (FAILED(hr)) {
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    //hr = pTriggerCollection->Create(pTrigger);
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pTriggerCollection->Release();
    pLogonTrigger->Release();
    pTrigger->Release();

    // Set the task to run with highest privileges
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr) && pPrincipal != NULL) {
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }
    else {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    // Register the task
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(L"Driver Starter"),  // Task name
        pTask,  // Task definition
        TASK_CREATE_OR_UPDATE,  // Create or update the task
        _variant_t(),  // No user account information
        _variant_t(),  // No password information
        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
        _variant_t(L""),  // No sddl security descriptor information
        &pRegisteredTask  // Task registration
    );
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pRegisteredTask->Release();
    pTask->Release();
    pRootFolder->Release();
    pService->Release();

    CoUninitialize();
}
bool removeTask()
{
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        return false;
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        CoUninitialize();
        return false;
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        CoUninitialize();
        return false;
    }

    hr = pRootFolder->DeleteTask(_bstr_t(L"Driver Starter"), 0); // Remove the task named "Driver Starter"
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    if (FAILED(hr)) {
        return false;
    }

    return true;
}