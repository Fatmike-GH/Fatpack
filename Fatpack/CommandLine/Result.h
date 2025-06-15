#pragma once
#include <Windows.h>
#include "Option.h"

namespace CommandLine
{
  const DWORD MAX_LEN = 256;
  struct Result
  {
    LPWSTR InputFileName = nullptr;
    LPWSTR OutputFileName = nullptr;
    OPTION Option = OPTION::RESOURCE;
    bool IsValid = false;
    WCHAR ErrorMessage[MAX_LEN] = { 0 };
  };
}