#include "ResourcePacker.h"
#include "PackerUtils.h"
#include "..\Compressor\Compressor.h"
#include "..\..\Shared\ResourceLoader\ResourceLoader.h"

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
    return LoadInputFile(inputFileName) &&
           PrepareLoaderStub() &&
           RebaseIfNeeded() &&
           SaveLoader(outputFileName) &&
           AddResources(inputFileName, outputFileName) &&
           CompressAndEmbed(outputFileName);
  }

  bool ResourcePacker::LoadInputFile(LPWSTR fileName)
  {
    return _packerUtils->ReadPeFile(fileName, _inputFile, _console) &&
           _packerUtils->ValidatePeFile(_inputFile, _console);
  }

  bool ResourcePacker::PrepareLoaderStub()
  {
    ResourceLoader::ResourceLoader resourceLoader;
    DWORD size = 0;
    BYTE* buffer = nullptr;

    if (_inputFile.IsConsole())
    {
      _console.WriteLine(L"Using console loader stub.");
      buffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1000), RT_RCDATA, size);
    }
    else
    {
      _console.WriteLine(L"Using windows loader stub.");
      buffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1001), RT_RCDATA, size);
    }

    if (!buffer || size == 0 || !_peLoader.LoadFromBuffer(buffer, size))
    {
      _console.WriteError(L"Loading loader stub failed.");
      return false;
    }

    resourceLoader.Free(buffer);
    return true;
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

  bool ResourcePacker::SaveLoader(LPWSTR fileName)
  {
    return _packerUtils->SavePeFile(fileName, _peLoader, _console);
  }

  bool ResourcePacker::AddResources(LPWSTR inputFileName, LPWSTR outputFileName)
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