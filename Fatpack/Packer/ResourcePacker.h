#pragma once
#include <Windows.h>
#include "..\PEFile\PEFile.h"
#include "..\Console\Console.h"

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

    PackerUtils* _packerUtils;
    PEFile::PEFile _peLoader;
    PEFile::PEFile _inputFile;
    Console::Console _console;
  };
}
