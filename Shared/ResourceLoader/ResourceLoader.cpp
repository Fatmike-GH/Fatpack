#include "ResourceLoader.h"

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
    HMODULE moduleHandle = ::GetModuleHandle(nullptr);
    HRSRC resourceHandle = ::FindResource(moduleHandle, resourceName, resourceType);
    return resourceHandle;
  }

  BYTE* ResourceLoader::LoadResource(LPCWSTR resourceName, LPCWSTR resourceType, DWORD& resourceSize)
  {
    resourceSize = 0;

    HMODULE moduleHandle = ::GetModuleHandle(nullptr);
    HRSRC resourceHandle = ::FindResource(moduleHandle, resourceName, resourceType);
    if (resourceHandle == nullptr) return nullptr;

    resourceSize = ::SizeofResource(moduleHandle, resourceHandle);
    if (resourceSize == 0) return nullptr;

    HGLOBAL resourceDataHandle = ::LoadResource(moduleHandle, resourceHandle);
    if (resourceDataHandle == nullptr) return nullptr;

    LPVOID resourceData = ::LockResource(resourceDataHandle);
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
