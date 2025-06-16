#include "PackerUtils.h"
#include "..\BinaryFileWriter\BinaryFileWriter.h"
#include "..\IconExtractor\IconExtractor.h"
#include "..\ManifestExtractor\ManifestExtractor.h"
#include "..\..\Shared\BinaryFileReader\BinaryFileReader.h"
#include "..\..\Shared\ResourceLoader\ResourceLoader.h"

namespace Packer
{
  PackerUtils::PackerUtils()
  {
  }

  PackerUtils::~PackerUtils()
  {
  }

  bool PackerUtils::ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile, Console::Console& console)
  {
    BinaryFileReader::BinaryFileReader reader(fileName);
    if (reader.GetBufferSize() == 0 || reader.GetBuffer() == nullptr)
    {
      console.WriteError(L"Failed to read file.");
      return false;
    }

    if (!peFile.LoadFromBuffer(reader.GetBuffer(), reader.GetBufferSize()))
    {
      console.WriteError(L"Failed to load PE file from buffer.");
      return false;
    }
    return true;
  }

  bool PackerUtils::ValidatePeFile(PEFile::PEFile& peFile, Console::Console& console)
  {
    if (!peFile.IsPEFile())
    {
      console.WriteError(L"Input file is not a valid PE file.");
      return false;
    }
    if (!peFile.Isx64())
    {
      console.WriteError(L"Input file is not a x64 PE file.");
      return false;
    }
    if (!peFile.IsNative())
    {
      console.WriteError(L"Input file is not a native PE file.");
      return false;
    }
    return true;
  }

  bool PackerUtils::PrepareLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader, Console::Console& console)
  {
    ResourceLoader::ResourceLoader resourceLoader;
    DWORD size = 0;
    BYTE* buffer = nullptr;

    if (inputFile.IsConsole())
    {
      console.WriteLine(L"Using console loader stub.");
      buffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1000), RT_RCDATA, size);
    }
    else
    {
      console.WriteLine(L"Using windows loader stub.");
      buffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1001), RT_RCDATA, size);
    }

    if (!buffer || size == 0 || !peLoader.LoadFromBuffer(buffer, size))
    {
      console.WriteError(L"Loading loader stub failed.");
      return false;
    }

    resourceLoader.Free(buffer);
    return true;
  }

  bool PackerUtils::SavePeFile(LPWSTR fileName, PEFile::PEFile& peFile, Console::Console& console)
  {
    BinaryFileWriter::BinaryFileWriter writer;
    if (!writer.WriteFile(fileName, peFile.GetBuffer(), peFile.GetBufferSize()))
    {
      console.WriteError(L"Failed to save PE file.");
      return false;
    }
    return true;
  }

  bool PackerUtils::AppendResources(LPWSTR inputFileName, LPWSTR outputFileName, Console::Console& console)
  {
    IconExtractor::IconExtractor iconExtractor;
    if (iconExtractor.ExtractAndSetIconWithCustomIds(inputFileName, outputFileName))
    {
      console.WriteLine(L"Icon added.");
    }

    ManifestExtractor::ManifestExtractor manifestExtractor;
    if (manifestExtractor.ExtractManifestResources(inputFileName))
    {
      console.WriteLine(L"Manifest found. Adding manifest...");
      if (!manifestExtractor.AddManifestResourcesToTarget(outputFileName))
      {
        console.WriteError(L"Adding manifest failed.");
        return false;
      }
      console.WriteLine(L"Manifest added.");
    }

    return true;
  }
}