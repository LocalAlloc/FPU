#include "CheckDriver.h"

#pragma warning(disable: 28251)

#define ID_START_SERVICE 1001
#define ID_STOP_SERVICE 1002
#define ID_INSTALL_SERVICE 1003
#define IDC_FILE_PANE 1004
#define ID_PROTECT_DIRECTORY 1005
#define ID_PROTECT_FILE 1006
#define ID_DRIVER_SETUP_WINDOW 1007
#define ID_UNINSTALL_SERVICE 1008
HINSTANCE g_hInstance;
int g_nCmdShow;
const wchar_t* driverName = L"FileProtector";
LRESULT CALLBACK DriverWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Handle the button click events
        switch (wmId)
        {
        case ID_START_SERVICE:
            StartDriver(driverName);
            break;
        case ID_STOP_SERVICE:
            StopDriver(driverName);
            break;
        case ID_INSTALL_SERVICE:
            InstallDriver();
            break;
        case ID_UNINSTALL_SERVICE:
            UninstallDriver(driverName);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
    {
        CreateWindow(
            L"BUTTON", L"Start Service",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            10, 10, 100, 30,
            hWnd, (HMENU)ID_START_SERVICE, GetModuleHandle(NULL), NULL
        );

        CreateWindow(
            L"BUTTON", L"Install Service",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            120, 10, 100, 30,
            hWnd, (HMENU)ID_INSTALL_SERVICE, GetModuleHandle(NULL), NULL
        );

        CreateWindow(
            L"BUTTON", L"Stop Service",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            230, 10, 100, 30,
            hWnd, (HMENU)ID_STOP_SERVICE, GetModuleHandle(NULL), NULL
        );

        CreateWindow(
            L"BUTTON", L"Uninstall Service",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            340, 10, 120, 30,
            hWnd, (HMENU)ID_UNINSTALL_SERVICE, GetModuleHandle(NULL), NULL
        );

        // Set the background color to light blue
        HBRUSH hBrush = CreateSolidBrush(RGB(173, 216, 230)); // RGB values for light blue
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void InitDriverWindow(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"DriverWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = DriverWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    InitCommonControls(); // Initialize Common Controls library

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Driver Management",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 100,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL)
    {
        MessageBoxA(NULL, "Failed to create window!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hFilePane;

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Handle the button click events
        switch (wmId)
        {
        case ID_DRIVER_SETUP_WINDOW:
            InitDriverWindow(g_hInstance, g_nCmdShow);
            break;
        case ID_PROTECT_DIRECTORY:
        {
            //StartDriver(driverName);
            if (!IsDriverRunning(driverName)) {
                MessageBoxA(NULL, "Dude Stop Pressing Me Please It Hurts!", "ERROR", MB_OK | MB_ICONSTOP);
                break;
            }
            TCHAR folderPath[MAX_PATH] = { 0 };

            BROWSEINFO browseInfo = { 0 };
            browseInfo.hwndOwner = hWnd;
            browseInfo.lpszTitle = _T("Select a Folder");
            browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

            LPITEMIDLIST folderItemIdList = SHBrowseForFolder(&browseInfo);
            if (folderItemIdList != NULL)
            {
                SHGetPathFromIDList(folderItemIdList, folderPath);
                CoTaskMemFree(folderItemIdList);

                // Rename the folder
                std::wstring protectedFolderPath = folderPath;
                protectedFolderPath += L"_protected";
                if (MoveFile(folderPath, protectedFolderPath.c_str()) == 0)
                {
                    MessageBoxA(NULL, "Failed to rename the folder!", NULL, MB_OK | MB_ICONSTOP);
                    break;
                }
                SetWindowText(hFilePane, folderPath);

                // Hide the folder
                DWORD attributes = GetFileAttributes(protectedFolderPath.c_str());
                if (attributes != INVALID_FILE_ATTRIBUTES)
                {
                    attributes |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
                    if (!SetFileAttributes(protectedFolderPath.c_str(), attributes))
                    {
                        MessageBoxA(NULL, "Failed to hide the folder!", NULL, MB_OK | MB_ICONSTOP);
                    }
                    else {
                        MessageBoxA(NULL, "The Folder Is Hidden!, Make Sure To Enable Hidden Files With System Attributes To View The Folders And Files", "Warning", MB_OK | MB_ICONWARNING);
                    }
                }
                else
                {
                    MessageBoxA(NULL, "Failed to get folder attributes!", NULL, MB_OK | MB_ICONSTOP);
                }
                FILE* FileProtector;
                if (FileProtector = fopen("C:\\Windows\\System32\\FileProtector.exe", "r")) {
                    break;
                }
                if (MessageBoxA(NULL, "You have decided to protect a folder.\nA driver is responsible for the folder being protected.\n\nThe driver must run on startup to constantly work and protect the folder.\n\nDo you want the driver to run on startup?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    char system[MAX_PATH];
                    char pathtofile[MAX_PATH];
                    HMODULE GetModH = GetModuleHandleA(NULL);
                    GetModuleFileNameA(GetModH, pathtofile, sizeof(pathtofile));
                    GetSystemDirectoryA(system, sizeof(system));
                    strcat_s(system, "\\FileProtector.exe");
                    CopyFileA(pathtofile, system, false);
                    setTask();
                    MessageBoxA(NULL, "Done Driver Successfully Scheduled To Run On Startup!", "Done", MB_OK | MB_ICONINFORMATION);
                    break;
                }
                else {
                    MessageBoxA(NULL, "You Have Selected The Driver To Not Run On Startup", "Conformation", MB_OK | MB_ICONINFORMATION);
                    break;
                }
            }
            break;
        }
        case ID_PROTECT_FILE:
        {
            if (!IsDriverRunning(driverName)) {
                MessageBoxA(NULL, "Dude Stop Pressing Me Please It Hurts!", "ERROR", MB_OK | MB_ICONSTOP);
                break;
            }

            TCHAR filePath[MAX_PATH] = { 0 };

            OPENFILENAME openFileName = { 0 };
            openFileName.lStructSize = sizeof(OPENFILENAME);
            openFileName.hwndOwner = hWnd;
            openFileName.lpstrFile = filePath;
            openFileName.nMaxFile = MAX_PATH;
            openFileName.lpstrTitle = _T("Select a File");
            openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

            if (GetOpenFileName(&openFileName))
            {
                SetWindowText(hFilePane, filePath);

                // Rename the file
                std::wstring protectedFilePath = filePath;
                size_t dotPos = protectedFilePath.rfind('.');
                if (dotPos != std::wstring::npos)
                {
                    protectedFilePath.insert(dotPos, L"_protected");
                    if (MoveFile(filePath, protectedFilePath.c_str()) == 0)
                    {
                        MessageBoxA(NULL, "Failed to rename the file!", NULL, MB_OK | MB_ICONSTOP);
                        break;
                    }

                    // Hide the file
                    DWORD attributes = GetFileAttributes(protectedFilePath.c_str());
                    if (attributes != INVALID_FILE_ATTRIBUTES)
                    {
                        attributes |= FILE_ATTRIBUTE_HIDDEN;
                        if (!SetFileAttributes(protectedFilePath.c_str(), attributes))
                        {
                            MessageBoxA(NULL, "Failed to hide the file!", NULL, MB_OK | MB_ICONSTOP);
                        }
                        else {
                            MessageBoxA(NULL, "The File Is Hidden!, Make Sure To Enable Hidden Files With System Attributes To View The Folders And Files", "Warning", MB_OK | MB_ICONWARNING);
                        }
                    }
                    else
                    {
                        MessageBoxA(NULL, "Failed to get file attributes!", NULL, MB_OK | MB_ICONSTOP);
                    }
                }
                else
                {
                    MessageBoxA(NULL, "Invalid file name!", NULL, MB_OK | MB_ICONSTOP);
                }
                FILE* FileProtector;
                if (FileProtector = fopen("C:\\Windows\\System32\\FileProtector.exe", "r")) {
                    break;
                }
                if (MessageBoxA(NULL, "You have decided to protect a file.\nA driver is responsible for the file being protected.\n\nThe driver must run on startup to constantly work and protect the folder.\n\nDo you want the driver to run on startup?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    char system[MAX_PATH];
                    char pathtofile[MAX_PATH];
                    HMODULE GetModH = GetModuleHandleA(NULL);
                    GetModuleFileNameA(GetModH, pathtofile, sizeof(pathtofile));
                    GetSystemDirectoryA(system, sizeof(system));
                    strcat_s(system, "\\FileProtector.exe");
                    CopyFileA(pathtofile, system, false);
                    setTask();
                    MessageBoxA(NULL, "Done Driver Successfully Scheduled To Run On Startup!", "Done", MB_OK | MB_ICONINFORMATION);
                    break;
                }
                else {
                    MessageBoxA(NULL, "You Have Selected The Driver To Not Run On Startup", "Conformation", MB_OK | MB_ICONINFORMATION);
                    break;
                }
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
    {
        hFilePane = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            _T("EDIT"),
            _T(""),
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 460, 200, // Adjust the coordinates and size of the edit pane
            hWnd,
            (HMENU)IDC_FILE_PANE,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            L"BUTTON", L"Protect Directory",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            10, 320, 140, 30, // Adjust the coordinates of the Protect Directory button
            hWnd, (HMENU)ID_PROTECT_DIRECTORY, GetModuleHandle(NULL), NULL
        );

        CreateWindow(
            L"BUTTON", L"Protect File",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            180, 320, 140, 30, // Adjust the coordinates of the Protect File button
            hWnd, (HMENU)ID_PROTECT_FILE, GetModuleHandle(NULL), NULL
        );

        CreateWindow(
            L"BUTTON", L"Driver Setup",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Add the BS_DEFPUSHBUTTON style for default button appearance
            340, 320, 140, 30, // Adjust the coordinates of the Driver Setup button
            hWnd, (HMENU)ID_DRIVER_SETUP_WINDOW, GetModuleHandle(NULL), NULL
        );
        // Set the background color to light blue
        HBRUSH hBrush = CreateSolidBrush(RGB(173, 216, 230)); // RGB values for light blue
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

        SetWindowTheme(hWnd, L"Explorer", NULL);

        break;
    }
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Resize the file pane to fit the window
        SetWindowPos(hFilePane, NULL, 10, 10, width - 20, height - 70, SWP_SHOWWINDOW); // Adjust the size and position of the edit pane
        break;
    }
    case WM_DESTROY:
        MessageBoxA(NULL, "Thanks For Using My Program!", "Thanks!", MB_OK | MB_ICONINFORMATION);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    MessageBoxA(NULL, "Remember To Stop The Driver To Modify Protected Files Or Folders!", "Warning!", MB_OK | MB_ICONWARNING);
    const wchar_t CLASS_NAME[] = L"FileProtectorWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    InitCommonControls(); // Initialize Common Controls library
    g_hInstance = hInstance;
    g_nCmdShow = nCmdShow;
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"FileProtector",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL)
    {
        MessageBoxA(NULL, "Failed to create window!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


