#pragma once
#include <Windows.h>

namespace ApiCaller
{
  class ApiCaller
  {
  public:
    static ApiCaller& Instance();

    HRSRC CallFindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);
    HMODULE CallGetModuleHandle(LPCSTR lpModuleName);
    FARPROC CallGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    HANDLE CallGetProcessHeap();
    LPVOID CallHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
    BOOL CallHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
    LPVOID CallHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
    HMODULE CallLoadLibrary(LPCSTR lpLibFileName);
    HGLOBAL CallLoadResource(HMODULE hModule, HRSRC hResInfo);
    LPVOID CallLockResource(HGLOBAL hResData);
    BOOLEAN CallRtlAddFunctionTable(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
    DWORD CallSizeofResource(HMODULE hModule, HRSRC hResInfo);
    LPVOID CallVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
    BOOL CallVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
    BOOL CallVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

  private:
    ApiCaller();
    ~ApiCaller();

    FARPROC ResolveKernel32Api(LPCSTR procName);

  private:
    static ApiCaller* _instance;
    HMODULE _kernel32;
  };
}
