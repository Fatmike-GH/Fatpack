#pragma once
#include <Windows.h>
#include "Error.h"
#include "..\PEFile\PEFile.h"

namespace Packer
{
  class PackerUtils
  {
  public:
    PackerUtils();
    ~PackerUtils();

    bool ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool ValidatePeFile(PEFile::PEFile& peFile);
    bool PrepareLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
    bool SavePeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool AppendResources(LPWSTR inputFileName, LPWSTR outputFileName);

    Error GetLastError() { return _lastError; }
    void SetLastError(Error error) { _lastError = error; }

  private:
    Error _lastError;
  };
}