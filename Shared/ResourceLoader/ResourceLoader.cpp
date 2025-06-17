#include "ResourceLoader.h"
#include "..\ApiCaller\ApiCaller.h"

namespace ResourceLoader
{
  ResourceLoader::ResourceLoader()
  {
  }

  ResourceLoader::~ResourceLoader()
  {
  }

  HRSRC ResourceLoader::FindResource(LPCWSTR resourceName, LPCWSTR resourceType)
  {
    HMODULE moduleHandle = ApiCaller::ApiCaller::Instance().CallGetModuleHandle(nullptr);
    HRSRC resourceHandle = ApiCaller::ApiCaller::Instance().CallFindResourceW(moduleHandle, resourceName, resourceType);
    return resourceHandle;
  }

  BYTE* ResourceLoader::LoadResource(LPCWSTR resourceName, LPCWSTR resourceType, DWORD& resourceSize)
  {
    resourceSize = 0;

    HMODULE moduleHandle = ApiCaller::ApiCaller::Instance().CallGetModuleHandle(nullptr);
    HRSRC resourceHandle = ApiCaller::ApiCaller::Instance().CallFindResourceW(moduleHandle, resourceName, resourceType);
    if (resourceHandle == nullptr) return nullptr;

    resourceSize = ApiCaller::ApiCaller::Instance().CallSizeofResource(moduleHandle, resourceHandle);
    if (resourceSize == 0) return nullptr;

    HGLOBAL resourceDataHandle = ApiCaller::ApiCaller::Instance().CallLoadResource(moduleHandle, resourceHandle);
    if (resourceDataHandle == nullptr) return nullptr;

    LPVOID resourceData = ApiCaller::ApiCaller::Instance().CallLockResource(resourceDataHandle);
    if (resourceData == nullptr) return nullptr;

    BYTE* buffer = new BYTE[resourceSize];
    memcpy(buffer, resourceData, resourceSize);
    return buffer;
  }

  void ResourceLoader::Free(BYTE* resourceBuffer)
  {
    if (resourceBuffer == nullptr) return;
    delete[] resourceBuffer;
  }
}
