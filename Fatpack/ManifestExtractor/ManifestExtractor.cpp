#include <windows.h>
#include "ManifestExtractor.h"
#include "..\..\Shared\ApiCaller\ApiCaller.h"
namespace ManifestExtractor
{
  ManifestResource* ManifestExtractor::_manifestResources = nullptr;
  DWORD ManifestExtractor::_manifestCount = 0;

  ManifestExtractor::ManifestExtractor()
  {
    _heapHandle = ApiCaller::ApiCaller::Instance().CallGetProcessHeap();
    _manifestResources = (ManifestResource*)ApiCaller::ApiCaller::Instance().CallHeapAlloc(_heapHandle, HEAP_ZERO_MEMORY, MAX_MANIFEST_RESOURCES * sizeof(ManifestResource));
    _manifestCount = 0;
  }

  ManifestExtractor::~ManifestExtractor()
  {
    if (_manifestResources)
    {
      HeapFree(_heapHandle, 0, _manifestResources);
      _manifestResources = nullptr;
    }
    _manifestCount = 0;
  }

  bool ManifestExtractor::ExtractManifestResources(LPWSTR fileName)
  {
    _manifestCount = 0;

    HMODULE sourceModuleHandle = LoadLibraryExW(fileName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!sourceModuleHandle)
    {
      return false;
    }

    if (!EnumResourceNames(sourceModuleHandle, MANIFEST_RESOURCE_TYPE, EnumResNameProc, 0))
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    return (_manifestCount > 0);
  }

  bool ManifestExtractor::AddManifestResourcesToTarget(LPWSTR destinationPath)
  {
    HANDLE updateHandle = BeginUpdateResourceW(destinationPath, FALSE);
    if (!updateHandle)
    {
      return false;
    }

    for (DWORD i = 0; i < _manifestCount; ++i)
    {
      ManifestResource& res = _manifestResources[i];
      if (!UpdateResource(updateHandle, MANIFEST_RESOURCE_TYPE, MAKEINTRESOURCE(res.id), res.lang, res.data, res.size))
      {
        EndUpdateResource(updateHandle, TRUE);
        return false;
      }
    }

    if (!EndUpdateResource(updateHandle, FALSE))
    {
      return false;
    }

    return true;
  }

  BOOL CALLBACK ManifestExtractor::EnumResNameProc(HMODULE moduleHandle, LPCTSTR type, LPTSTR name, LONG_PTR param)
  {
    EnumResourceLanguages(moduleHandle, type, name, EnumResLangProc, param);
    return TRUE;
  }

  BOOL CALLBACK ManifestExtractor::EnumResLangProc(HMODULE moduleHandle, LPCTSTR type, LPCTSTR name, WORD language, LONG_PTR param)
  {
    if (_manifestCount >= MAX_MANIFEST_RESOURCES)
    {
      return FALSE; // Stop enumeration
    }

    HRSRC hRes = FindResourceEx(moduleHandle, type, name, language);
    if (!hRes) return TRUE;

    DWORD resourceSize = SizeofResource(moduleHandle, hRes);
    HGLOBAL resourceDataHandle = LoadResource(moduleHandle, hRes);
    LPVOID resourceData = LockResource(resourceDataHandle);

    if (resourceData && resourceSize > 0)
    {
      ManifestResource& res = _manifestResources[_manifestCount++];
      res.id = (WORD)(IS_INTRESOURCE(name) ? (WORD)(uintptr_t)name : 1);
      res.lang = language;
      res.size = resourceSize;
      res.data = resourceData;
    }

    return TRUE; // Continue enumeration
  }
}