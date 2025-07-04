#pragma once
#include <Windows.h>
#include "..\Error\Error.h"
#include "..\PEFile\PEFile.h"

namespace Packer
{
  class PackerUtils;
}

namespace Packer
{
  class SectionPacker
  {
  public:
    SectionPacker(PackerUtils* packerUtils);
    ~SectionPacker();

    bool Pack(LPWSTR inputFileName, LPWSTR outputFileName);
    Error::ErrorCode GetLastError() { return _lastError; }

  private:
    bool ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool ValidateInputFile();
    bool PrepareLoaderStub();
    bool SavePeLoader(LPWSTR outputFileName);
    bool AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName);
    bool RebaseLoaderToLastSection();
    bool CompressAndAppendToLastSection();
    void SetLastError(Error::ErrorCode error) { _lastError = error; }

  private:
    PackerUtils* _packerUtils;
    PEFile::PEFile _peLoader;
    PEFile::PEFile _inputFile;
    Error::ErrorCode _lastError;
  };
}