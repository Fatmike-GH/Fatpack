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
    bool PrepareResourceLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
    bool PrepareSectionLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
    
    bool SavePeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool AppendResources(LPWSTR inputFileName, LPWSTR outputFileName);
    Error::ErrorCode GetLastError() { return _lastError; }

  private:
    bool PrepareLoaderStub(PEFile::PEFile& peLoader, WORD resourceId);
    void SetLastError(Error::ErrorCode error) { _lastError = error; }

  private:
    Error::ErrorCode _lastError;
  };
}