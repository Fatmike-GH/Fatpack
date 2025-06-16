#pragma once
#include <Windows.h>
#include "..\Error\Error.h"
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
    Error::ErrorCode GetLastError() { return _lastError; }

  private:
    void SetLastError(Error::ErrorCode error) { _lastError = error; }

  private:
    Error::ErrorCode _lastError;
  };
}