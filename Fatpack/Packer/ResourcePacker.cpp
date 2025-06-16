#include "ResourcePacker.h"
#include "PackerUtils.h"
#include "..\Compressor\Compressor.h"

namespace Packer
{
  ResourcePacker::ResourcePacker(PackerUtils* packerUtils)
  {
    _packerUtils = packerUtils;
  }

  ResourcePacker::~ResourcePacker()
  {
  }

  bool ResourcePacker::Pack(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    return ReadPeFile(inputFileName, _inputFile) &&
           ValidateInputFile() &&
           PrepareLoaderStub() &&
           RebaseIfNeeded() &&
           SavePeLoader(outputFileName) &&
           AppendResourcesToLoader(inputFileName, outputFileName) &&
           CompressAndEmbed(outputFileName);
  }

  bool ResourcePacker::ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile)
  {
    return _packerUtils->ReadPeFile(fileName, peFile, _console);
  }

  bool ResourcePacker::ValidateInputFile()
  {
    return _packerUtils->ValidatePeFile(_inputFile, _console);
  }

  bool ResourcePacker::PrepareLoaderStub()
  {
    return _packerUtils->PrepareLoaderStub(_inputFile, _peLoader, _console);
  }

  bool ResourcePacker::RebaseIfNeeded()
  {
    if (!_inputFile.HasRelocationTable() && _inputFile.IntersectsWith(_peLoader))
    {
      _console.WriteLine(L"Image base conflict. Rebasing loader...");
      ULONGLONG newBase = _inputFile.GetNextImageBase();
      if (!_peLoader.Rebase(newBase))
      {
        _console.WriteError(L"Rebasing failed.");
        return false;
      }
      _console.WriteLine(L"Rebasing finished.");
    }
    return true;
  }

  bool ResourcePacker::SavePeLoader(LPWSTR fileName)
  {
    return _packerUtils->SavePeFile(fileName, _peLoader, _console);
  }

  bool ResourcePacker::AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    return _packerUtils->AppendResources(inputFileName, outputFileName, _console);
  }

  bool ResourcePacker::CompressAndEmbed(LPWSTR outputFileName)
  {
    Compressor::Compressor compressor;
    BYTE* compressed = nullptr;
    size_t compressedSize = 0;

    if (!compressor.Compress(_inputFile.GetBuffer(), _inputFile.GetBufferSize(), &compressed, &compressedSize))
    {
      _console.WriteError(L"Compression failed.");
      return false;
    }

    HANDLE updateHandle = BeginUpdateResourceW(outputFileName, FALSE);
    if (updateHandle == nullptr)
    {
      _console.WriteError(L"Appending compressed data failed.");
      return false;
    }
    if (UpdateResource(updateHandle, RT_RCDATA, L"FPACK", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), compressed, (DWORD)compressedSize) == FALSE)
    {
      _console.WriteError(L"Appending compressed data failed.");
      return false;
    }
    if (EndUpdateResource(updateHandle, FALSE) == FALSE)
    {
      _console.WriteError(L"Appending compressed data failed.");
      return false;
    }

    compressor.Free(compressed);

    return true;
  }
}