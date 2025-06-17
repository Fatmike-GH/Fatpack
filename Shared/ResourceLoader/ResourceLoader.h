#pragma once
#include <Windows.h>

namespace ResourceLoader
{
  class ResourceLoader
  {
  public:
    ResourceLoader();
    ~ResourceLoader();

    BYTE* LoadResource(LPCWSTR resourceName, LPCWSTR resourceType, DWORD& resourceSize);
    void Free(BYTE* resourceBuffer);
  };
}