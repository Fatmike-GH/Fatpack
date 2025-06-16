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
  class SectionPacker
  {
  public:
    SectionPacker(PackerUtils* packerUtils);
    ~SectionPacker();

    bool Pack(LPWSTR inputFileName, LPWSTR outputFileName);

  private:
    bool ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile);
    bool ValidateInputFile();
    bool PrepareLoaderStub();
    bool SavePeLoader(LPWSTR outputFileName);
    bool AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName);
    bool RebaseLoaderToLastSection();
    bool CompressAndAppendToLastSection();

    PackerUtils* _packerUtils;
    PEFile::PEFile _peLoader;
    PEFile::PEFile _inputFile;
    Console::Console _console;
  };
}