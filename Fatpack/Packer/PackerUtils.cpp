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
    _lastError = Error::ErrorCode::Ok;
  }

  PackerUtils::~PackerUtils()
  {
  }

  bool PackerUtils::ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile)
  {
    BinaryFileReader::BinaryFileReader reader(fileName);
    if (reader.GetBufferSize() == 0 || reader.GetBuffer() == nullptr)
    {
      SetLastError(Error::ErrorCode::Error_Read_File);
      return false;
    }

    if (!peFile.LoadFromBuffer(reader.GetBuffer(), reader.GetBufferSize()))
    {
      SetLastError(Error::ErrorCode::Error_Load_Pe_From_Buffer);
      return false;
    }
    return true;
  }

  bool PackerUtils::ValidatePeFile(PEFile::PEFile& peFile)
  {
    if (!peFile.IsPEFile())
    {
      SetLastError(Error::ErrorCode::Error_Pe_Not_Valid);
      return false;
    }
    if (!peFile.Isx64())
    {
      SetLastError(Error::ErrorCode::Error_Pe_Not_x64);
      return false;
    }
    if (!peFile.IsNative())
    {
      SetLastError(Error::ErrorCode::Error_Pe_Not_Native);
      return false;
    }
    return true;
  }

  bool PackerUtils::PrepareResourceLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
  {
    if (inputFile.IsConsole())
    {
      return PrepareLoaderStub(peLoader, 1000);
    }
    else
    {
      return PrepareLoaderStub(peLoader, 1001);
    }
  }

  bool PackerUtils::PrepareSectionLoaderStub(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
  {
    if (inputFile.IsConsole())
    {
      return PrepareLoaderStub(peLoader, 1002);
    }
    else
    {
      return PrepareLoaderStub(peLoader, 1003);
    }
  }

  bool PackerUtils::PrepareLoaderStub(PEFile::PEFile& peLoader, WORD resourceId)
  {
    ResourceLoader::ResourceLoader resourceLoader;
    DWORD size = 0;
    BYTE* buffer = nullptr;

    buffer = resourceLoader.LoadResource(MAKEINTRESOURCE(resourceId), RT_RCDATA, size);

    if (!buffer || size == 0 || !peLoader.LoadFromBuffer(buffer, size))
    {
      SetLastError(Error::ErrorCode::Error_Loading_Loader_Stub);
      return false;
    }

    resourceLoader.Free(buffer);
    return true;
  }

  bool PackerUtils::SavePeFile(LPWSTR fileName, PEFile::PEFile& peFile)
  {
    BinaryFileWriter::BinaryFileWriter writer;
    if (!writer.WriteFile(fileName, peFile.GetBuffer(), peFile.GetBufferSize()))
    {
      SetLastError(Error::ErrorCode::Error_Saving_Pe);
      return false;
    }
    return true;
  }

  bool PackerUtils::AppendResources(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    IconExtractor::IconExtractor iconExtractor;
    if (!iconExtractor.ExtractAndSetIconWithCustomIds(inputFileName, outputFileName))
    {
      SetLastError(Error::ErrorCode::Error_Adding_Icon);
      return false;
    }

    ManifestExtractor::ManifestExtractor manifestExtractor;
    if (manifestExtractor.ExtractManifestResources(inputFileName))
    {
      if (!manifestExtractor.AddManifestResourcesToTarget(outputFileName))
      {
        SetLastError(Error::ErrorCode::Error_Adding_Manifest);
        return false;
      }
    }

    return true;
  }
}