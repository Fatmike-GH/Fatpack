#include "IconExtractor.h"

namespace IconExtractor
{
#pragma pack(push, 1)
  typedef struct
  {
    BYTE  Width, Height, ColorCount, Reserved;
    WORD  Planes, BitCount;
    DWORD BytesInRes;
    WORD  ID;
  } ICONENTRY;

  typedef struct
  {
    WORD Reserved, Type, Count;
    ICONENTRY Entries[1]; // Variable length
  } ICONGROUP;
#pragma pack(pop)

  IconExtractor::IconExtractor()
  {
  }

  IconExtractor::~IconExtractor()
  {
  }

  bool IconExtractor::ExtractAndSetIcon(LPWSTR sourceExe, LPWSTR targetExe)
  {
    HMODULE sourceModuleHandle = LoadLibraryExW(sourceExe, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!sourceModuleHandle)
    {
      return false;
    }

    LPCWSTR groupIconName = nullptr;
    EnumResourceNames(sourceModuleHandle, RT_GROUP_ICON, EnumResNameProc, (LONG_PTR)&groupIconName);

    if (!groupIconName)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    HRSRC groupIconHandle = FindResource(sourceModuleHandle, groupIconName, RT_GROUP_ICON);
    if (!groupIconHandle)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    HGLOBAL groupIconDataHandle = LoadResource(sourceModuleHandle, groupIconHandle);
    if (!groupIconDataHandle)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    LPVOID groupIconData = LockResource(groupIconDataHandle);
    DWORD groupIconSize = SizeofResource(sourceModuleHandle, groupIconHandle);

    HANDLE updateHandle = BeginUpdateResourceW(targetExe, FALSE);
    if (!updateHandle)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    // Update the icon group resource
    if (!UpdateResource(updateHandle, RT_GROUP_ICON, groupIconName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), groupIconData, groupIconSize))
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    ICONGROUP* iconGroup = (ICONGROUP*)groupIconData;
    for (int i = 0; i < iconGroup->Count; i++)
    {
      WORD iconId = iconGroup->Entries[i].ID;
      HRSRC iconHandle = FindResource(sourceModuleHandle, MAKEINTRESOURCE(iconId), RT_ICON);
      if (iconHandle)
      {
        HGLOBAL iconDataHandle = LoadResource(sourceModuleHandle, iconHandle);
        if (iconDataHandle)
        {
          LPVOID iconData = LockResource(iconDataHandle);
          DWORD iconSize = SizeofResource(sourceModuleHandle, iconHandle);

          if (!UpdateResource(updateHandle, RT_ICON, MAKEINTRESOURCE(iconId), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), iconData, iconSize))
          {
            return false;
          }
        }
      }
    }

    // Finalize the update
    if (!EndUpdateResource(updateHandle, FALSE))
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    FreeLibrary(sourceModuleHandle);

    return true;
  }

  bool IconExtractor::ExtractAndSetIconWithCustomIds(LPWSTR sourceExe, LPWSTR targetExe)
  {
    HMODULE sourceModuleHandle = LoadLibraryExW(sourceExe, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!sourceModuleHandle)
    {
      return false;
    }

    LPCWSTR groupIconName = nullptr;
    EnumResourceNames(sourceModuleHandle, RT_GROUP_ICON, EnumResNameProc, (LONG_PTR)&groupIconName);

    if (!groupIconName)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    // Find and load the RT_GROUP_ICON resource
    HRSRC groupIconHandle = FindResource(sourceModuleHandle, groupIconName, RT_GROUP_ICON);
    if (!groupIconHandle)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    HGLOBAL groupIconDataHandle = LoadResource(sourceModuleHandle, groupIconHandle);
    if (!groupIconDataHandle)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    ICONGROUP* groupIcon = (ICONGROUP*)LockResource(groupIconDataHandle);
    DWORD groupIconSize = SizeofResource(sourceModuleHandle, groupIconHandle);

    // Create a copy of the RT_GROUP_ICON structure to modify it
    DWORD newGroupIconSize = sizeof(ICONGROUP) + (groupIcon->Count - 1) * sizeof(ICONENTRY);
    ICONGROUP* newGroupIcon = (ICONGROUP*)malloc(newGroupIconSize);
    if (!newGroupIcon)
    {
      FreeLibrary(sourceModuleHandle);
      return false;
    }

    // Copy original RT_GROUP_ICON data
    memcpy(newGroupIcon, groupIcon, newGroupIconSize);

    HANDLE updataHandle = BeginUpdateResourceW(targetExe, FALSE);
    if (!updataHandle)
    {
      FreeLibrary(sourceModuleHandle);
      free(newGroupIcon);
      return false;
    }

    // Update the icon group with a new ID
    if (!UpdateResource(updataHandle, RT_GROUP_ICON, MAKEINTRESOURCE(CUSTOM_ICON_GROUP_ID), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newGroupIcon, newGroupIconSize))
    {
      FreeLibrary(sourceModuleHandle);
      free(newGroupIcon);
      return false;
    }

    // Extract and update RT_ICON resources with new IDs
    for (int i = 0; i < groupIcon->Count; i++)
    {
      WORD oldIconId = groupIcon->Entries[i].ID;
      WORD newIconId = CUSTOM_ICON_BASE_ID + i;

      HRSRC oldIconHandle = FindResource(sourceModuleHandle, MAKEINTRESOURCE(oldIconId), RT_ICON);
      if (oldIconHandle)
      {
        HGLOBAL oldIconDataHandle = LoadResource(sourceModuleHandle, oldIconHandle);
        if (oldIconDataHandle)
        {
          LPVOID oldIconData = LockResource(oldIconDataHandle);
          DWORD oldIconSize = SizeofResource(sourceModuleHandle, oldIconHandle);

          if (UpdateResource(updataHandle, RT_ICON, MAKEINTRESOURCE(newIconId), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), oldIconData, oldIconSize))
          {
            // Modify the RT_GROUP_ICON to reference new IDs
            newGroupIcon->Entries[i].ID = newIconId;
          }
        }
      }
    }

    // Update RT_GROUP_ICON with corrected icon IDs
    if (!UpdateResource(updataHandle, RT_GROUP_ICON, MAKEINTRESOURCE(CUSTOM_ICON_GROUP_ID), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newGroupIcon, newGroupIconSize))
    {
      FreeLibrary(sourceModuleHandle);
      free(newGroupIcon);
      return false;
    }

    // Finalize the update
    if (!EndUpdateResource(updataHandle, FALSE))
    {
      FreeLibrary(sourceModuleHandle);
      free(newGroupIcon);
      return false;
    }

    FreeLibrary(sourceModuleHandle);
    free(newGroupIcon);

    return true;
  }

  BOOL CALLBACK IconExtractor::EnumResNameProc(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
  {
    if (lpType == RT_GROUP_ICON)
    {
      *((LPWSTR*)lParam) = lpName;
      return FALSE; // Stop after finding the first icon
    }
    return TRUE;
  }
}