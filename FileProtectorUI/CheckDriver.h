#include <Windows.h>
#include <iostream>
#include <string>
#include <ShlObj.h>
#include <tchar.h>
#include "resource.h"
#include <uxtheme.h>
#include <taskschd.h>
#include <comutil.h>

#pragma comment(lib, "Taskschd.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern BOOL IsDriverRunning(const wchar_t* driverName);
extern bool StopDriver(const std::wstring& driverName);
extern bool StartDriver(const std::wstring& driverName);
extern void InstallDriver();
extern void UninstallDriver(const std::wstring& driverName);
extern bool ExtractResourceExe(int iId, LPCWSTR pDest);
extern bool ExtractResourceDll(int iId, LPCWSTR pDest);
extern bool ExtractResourceSys(int iId, LPCWSTR pDest);
extern bool ExtractResourceInf(int iId, LPCWSTR pDest);
extern bool StartDriverStartup(const std::wstring& driverName);
extern bool removeTask();
extern int setTask();
//const wchar_t* driverName1 = L"FileProtector";