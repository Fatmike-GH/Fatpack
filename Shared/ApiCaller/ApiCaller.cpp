#include "ApiCaller.h"

namespace ApiCaller
{
  ApiCaller* ApiCaller::_instance = nullptr;

  ApiCaller::ApiCaller()
  {
    _kernel32 = CallLoadLibrary("kernel32.dll");
  }

  ApiCaller::~ApiCaller()
  {
    delete _instance;
  }

  ApiCaller& ApiCaller::Instance()
  {
    if (_instance == nullptr)
    {
      _instance = new ApiCaller();
    }    
    return *_instance;
  }

  HRSRC ApiCaller::CallFindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
  {
    auto function = (decltype(FindResourceW)*)ResolveKernel32Api("FindResourceW");
    return function(hModule, lpName, lpType);
  }

  HMODULE ApiCaller::CallGetModuleHandle(LPCSTR lpModuleName)
  {
    /*auto function = (decltype(GetModuleHandleA)*)ResolveKernel32Api("GetModuleHandleA");
    return function(lpModuleName);*/
    return GetModuleHandleA(lpModuleName);
  }

  FARPROC ApiCaller::CallGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
  {
    return GetProcAddress(hModule, lpProcName);
  }

  HANDLE ApiCaller::CallGetProcessHeap()
  {
    /*auto function = (decltype(GetProcessHeap)*)ResolveKernel32Api("GetProcessHeap");
    return function();*/
    return GetProcessHeap();
  }

  LPVOID ApiCaller::CallHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
  {
    /*auto function = (decltype(HeapAlloc)*)ResolveKernel32Api("HeapAlloc");
    return function(hHeap, dwFlags, dwBytes);*/
    return HeapAlloc(hHeap, dwFlags, dwBytes);
  }

  BOOL ApiCaller::CallHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
  {
    /*auto function = (decltype(HeapFree)*)ResolveKernel32Api("HeapFree");
    return function(hHeap, dwFlags, lpMem);*/
    return HeapFree(hHeap, dwFlags, lpMem);
  }

  LPVOID ApiCaller::CallHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes)
  {
    /*auto function = (decltype(HeapReAlloc)*)ResolveKernel32Api("HeapReAlloc");
    return function(hHeap, dwFlags, lpMem, dwBytes);*/
    return HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
  }

  HMODULE ApiCaller::CallLoadLibrary(LPCSTR lpLibFileName)
  {
    /*auto function = (decltype(LoadLibraryA)*)ResolveKernel32Api("LoadLibraryA");
    return function(lpLibFileName);*/
    return LoadLibraryA(lpLibFileName);
  }

  HGLOBAL ApiCaller::CallLoadResource(HMODULE hModule, HRSRC hResInfo)
  {
    auto function = (decltype(LoadResource)*)ResolveKernel32Api("LoadResource");
    return function(hModule, hResInfo);
  }

  LPVOID ApiCaller::CallLockResource(HGLOBAL hResData)
  {
    auto function = (decltype(LockResource)*)ResolveKernel32Api("LockResource");
    return function(hResData);
  }

  BOOLEAN ApiCaller::CallRtlAddFunctionTable(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress)
  {
    auto function = (decltype(RtlAddFunctionTable)*)ResolveKernel32Api("RtlAddFunctionTable");
    return function(FunctionTable, EntryCount, BaseAddress);
  }

  DWORD ApiCaller::CallSizeofResource(HMODULE hModule, HRSRC hResInfo)
  {
    auto function = (decltype(SizeofResource)*)ResolveKernel32Api("SizeofResource");
    return function(hModule, hResInfo);
  }

  LPVOID ApiCaller::CallVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
  {
    auto function = (decltype(VirtualAlloc)*)ResolveKernel32Api("VirtualAlloc");
    return function(lpAddress, dwSize, flAllocationType, flProtect);
  }

  BOOL ApiCaller::CallVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
  {
    auto function = (decltype(VirtualFree)*)ResolveKernel32Api("VirtualFree");
    return function(lpAddress, dwSize, dwFreeType);
  }

  BOOL ApiCaller::CallVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
  {
    auto function = (decltype(VirtualProtect)*)ResolveKernel32Api("VirtualProtect");
    return function(lpAddress, dwSize, flNewProtect, lpflOldProtect);
  }

  FARPROC ApiCaller::ResolveKernel32Api(LPCSTR procName)
  {
    return CallGetProcAddress(_kernel32, procName);
  }
}