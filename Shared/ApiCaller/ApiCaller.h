#pragma once
#include <Windows.h>

namespace ApiCaller
{
  class ApiCaller
  {
  public:
    static ApiCaller& Instance();

    __declspec(noinline) HRSRC CallFindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);
    __declspec(noinline) HMODULE CallGetModuleHandle(LPCSTR lpModuleName);
    __declspec(noinline) FARPROC CallGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    __declspec(noinline) HANDLE CallGetProcessHeap();
    __declspec(noinline) LPVOID CallHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
    __declspec(noinline) BOOL CallHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
    __declspec(noinline) LPVOID CallHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
    __declspec(noinline) HMODULE CallLoadLibrary(LPCSTR lpLibFileName);
    __declspec(noinline) HGLOBAL CallLoadResource(HMODULE hModule, HRSRC hResInfo);
    __declspec(noinline) LPVOID CallLockResource(HGLOBAL hResData);
    __declspec(noinline) BOOLEAN CallRtlAddFunctionTable(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
    __declspec(noinline) DWORD CallSizeofResource(HMODULE hModule, HRSRC hResInfo);
    __declspec(noinline) LPVOID CallVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
    __declspec(noinline) BOOL CallVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
    __declspec(noinline) BOOL CallVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

  private:
    ApiCaller();
    ~ApiCaller();

    FARPROC ResolveKernel32Api(LPCSTR procName);

  private:
    static ApiCaller* _instance;
    HMODULE _kernel32;
  };
}
