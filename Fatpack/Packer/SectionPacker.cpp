#include "SectionPacker.h"
#include "PackerUtils.h"
#include "..\Compressor\Compressor.h"

namespace Packer
{
  SectionPacker::SectionPacker(PackerUtils* packerUtils)
  {
    _packerUtils = packerUtils;
    _lastError = Error::ErrorCode::Ok;
  }

  SectionPacker::~SectionPacker()
  {
  }

  bool SectionPacker::Pack(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    SetLastError(Error::ErrorCode::Ok);

    return ReadPeFile(inputFileName, _inputFile) &&
           ValidateInputFile() &&
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
    if (!_packerUtils->ReadPeFile(fileName, peFile))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool SectionPacker::ValidateInputFile()
  {
    if (!_packerUtils->ValidatePeFile(_inputFile))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool SectionPacker::PrepareLoaderStub()
  {
    if (!_packerUtils->PrepareLoaderStub(_inputFile, _peLoader))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool SectionPacker::SavePeLoader(LPWSTR outputFileName)
  {
    if (!_packerUtils->SavePeFile(outputFileName, _peLoader))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool SectionPacker::AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    if (!_packerUtils->AppendResources(inputFileName, outputFileName))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool SectionPacker::RebaseLoaderToLastSection()
  {
    WORD lastIndex = _peLoader.GetSectionCount() - 1;
    PIMAGE_SECTION_HEADER lastSection = _peLoader.GetSectionHeader(lastIndex);
    uintptr_t newImageBase = _inputFile.GetImageBase() - lastSection->VirtualAddress;
    uintptr_t alignedBase = PEFile::PEFile::AlignImageBase(newImageBase);

    if (!_peLoader.Rebase(alignedBase))
    {
      SetLastError(Error::ErrorCode::Error_Rebasing);
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

    return true;
  }

  bool SectionPacker::CompressAndAppendToLastSection()
  {
    Compressor::Compressor compressor;
    BYTE* compressed = nullptr;
    size_t compressedSize = 0;

    if (!compressor.Compress(_inputFile.GetBuffer(), _inputFile.GetBufferSize(), &compressed, &compressedSize))
    {
      SetLastError(Error::ErrorCode::Error_Compressing);
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
      SetLastError(Error::ErrorCode::Error_Appending_Compressed_Data);
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