#pragma once
#include <Windows.h>
#include "..\PEFile\PEFile.h"
#include "..\Console\Console.h"

namespace Packer
{
  class PackerUtils
  {
  public:
    PackerUtils();
    ~PackerUtils();

    bool ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile, Console::Console& console);
    bool ValidatePeFile(PEFile::PEFile& peFile, Console::Console& console);
    bool SavePeFile(LPWSTR fileName, PEFile::PEFile& peFile, Console::Console& console);
    bool AppendResources(LPWSTR inputFileName, LPWSTR outputFileName, Console::Console& console);
  };
}