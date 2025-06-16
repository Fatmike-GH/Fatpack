#pragma once
#include <Windows.h>
#include "Error.h"
#include "..\PEFile\PEFile.h"

namespace Packer
{
  class PackerUtils;
}

namespace Packer
{
  class ResourcePacker
  {
  public:
    ResourcePacker(PackerUtils* packerUtils);
    ~ResourcePacker();

    bool Pack(LPWSTR inputFileName, LPWSTR outputFileName);

  private:
    bool ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool ValidateInputFile();
    bool PrepareLoaderStub();
    bool RebaseIfNeeded();
    bool SavePeLoader(LPWSTR fileName);
    bool AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName);
    bool CompressAndEmbed(LPWSTR outputFileName);

    Error GetLastError() { return _lastError; }
    void SetLastError(Error error) { _lastError = error; }

  private:
    PackerUtils* _packerUtils;
    PEFile::PEFile _peLoader;
    PEFile::PEFile _inputFile;
    Error _lastError;
  };
}
