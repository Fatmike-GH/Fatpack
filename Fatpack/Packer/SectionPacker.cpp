#include "SectionPacker.h"
#include "PackerUtils.h"
#include "..\Compressor\Compressor.h"
#include "..\..\Shared\ResourceLoader\ResourceLoader.h"

namespace Packer
{
  SectionPacker::SectionPacker(PackerUtils* packerUtils)
  {
    _packerUtils = packerUtils;
  }

  SectionPacker::~SectionPacker()
  {
  }

  bool SectionPacker::Pack(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    return ReadPeFile(inputFileName, _inputFile) &&
           ValidatePeFile(_inputFile) &&
           PrepareLoaderStub() &&
           SavePeLoader(outputFileName) &&
           AppendResourcesToLoader(inputFileName, outputFileName) &&
           ReadPeFile(outputFileName, _peLoader) &&
           RebaseLoaderToLastSection() &&
           CompressAndAppendToLastSection() &&
           SavePeLoader(outputFileName);
  }

  bool SectionPacker::ReadPeFile(LPWSTR fileName, PEFile::PEFile& peFile)
  {
    return _packerUtils->ReadPeFile(fileName, peFile, _console);
          
  }

  bool SectionPacker::ValidatePeFile(PEFile::PEFile& peFile)
  {
    return _packerUtils->ValidatePeFile(_inputFile, _console);
  }

  bool SectionPacker::PrepareLoaderStub()
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

  bool SectionPacker::SavePeLoader(LPWSTR outputFileName)
  {
    return _packerUtils->SavePeFile(outputFileName, _peLoader, _console);
  }

  bool SectionPacker::AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    return _packerUtils->AppendResources(inputFileName, outputFileName, _console);
  }

  bool SectionPacker::RebaseLoaderToLastSection()
  {
    WORD lastIndex = _peLoader.GetSectionCount() - 1;
    PIMAGE_SECTION_HEADER lastSection = _peLoader.GetSectionHeader(lastIndex);
    uintptr_t newImageBase = _inputFile.GetImageBase() - lastSection->VirtualAddress;
    uintptr_t alignedBase = PEFile::PEFile::AlignImageBase(newImageBase);

    if (!_peLoader.Rebase(alignedBase))
    {
      _console.WriteError(L"Rebasing failed.");
      return false;
    }

    PIMAGE_SECTION_HEADER prevSection = _peLoader.GetSectionHeader(lastIndex - 1);
    DWORD gap = (DWORD)(newImageBase - alignedBase);
    if (gap != 0)
    {
      prevSection->Misc.VirtualSize += gap;
    }

    lastSection->VirtualAddress = _peLoader.AlignSection(prevSection->VirtualAddress + prevSection->Misc.VirtualSize);
    lastSection->Misc.VirtualSize = _peLoader.AlignSection(_inputFile.GetSizeOfImage());
    _peLoader.NT_HEADERS()->OptionalHeader.SizeOfImage = _peLoader.AlignSection(lastSection->VirtualAddress + lastSection->Misc.VirtualSize);

    _console.WriteLine(L"Rebasing finished.");
    return true;
  }

  bool SectionPacker::CompressAndAppendToLastSection()
  {
    Compressor::Compressor compressor;
    BYTE* compressed = nullptr;
    size_t compressedSize = 0;

    if (!compressor.Compress(_inputFile.GetBuffer(), _inputFile.GetBufferSize(), &compressed, &compressedSize))
    {
      _console.WriteError(L"Compression failed.");
      return false;
    }

    WORD lastIndex = _peLoader.GetSectionCount() - 1;
    PIMAGE_SECTION_HEADER lastSection = _peLoader.GetSectionHeader(lastIndex);

    DWORD oldRawSize = lastSection->SizeOfRawData;
    DWORD newRawSize = _peLoader.AlignFile((DWORD)compressedSize);
    DWORD delta = newRawSize - oldRawSize;

    DWORD oldBufferSize = _peLoader.GetBufferSize();
    DWORD newBufferSize = oldBufferSize + delta;
    lastSection->SizeOfRawData = newRawSize;

    if (!_peLoader.Resize(newBufferSize))
    {
      _console.WriteError(L"Appending compressed data failed.");
      compressor.Free(compressed);
      return false;
    }

    lastSection = _peLoader.GetSectionHeader(lastIndex);
    memcpy(_peLoader.GetBuffer() + lastSection->PointerToRawData, compressed, compressedSize);
    memcpy(lastSection->Name, ".fpack  ", 8);

    compressor.Free(compressed);
    return true;
  }
}