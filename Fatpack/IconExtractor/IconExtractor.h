#pragma once
#include <Windows.h>

#define CUSTOM_ICON_GROUP_ID 100 // Custom ID for RT_GROUP_ICON
#define CUSTOM_ICON_BASE_ID  101 // Custom base ID for RT_ICON entries

namespace IconExtractor
{
  class IconExtractor
  {
  public:
    IconExtractor();
    ~IconExtractor();

    bool ExtractAndSetIcon(LPWSTR sourceExe, LPWSTR targetExe);
    bool ExtractAndSetIconWithCustomIds(LPWSTR sourceExe, LPWSTR targetExe);

    private:
      static BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam);
  };
}