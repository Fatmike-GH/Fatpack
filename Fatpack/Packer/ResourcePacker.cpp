#include "ResourcePacker.h"
#include "PackerUtils.h"
#include "..\Compressor\Compressor.h"

namespace Packer
{
  ResourcePacker::ResourcePacker(PackerUtils* packerUtils)
  {
    _packerUtils = packerUtils;
    _lastError = Error::ErrorCode::Ok;
  }

  ResourcePacker::~ResourcePacker()
  {
  }

  bool ResourcePacker::Pack(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    SetLastError(Error::ErrorCode::Ok);

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
    if (!_packerUtils->ReadPeFile(fileName, peFile))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool ResourcePacker::ValidateInputFile()
  {
    if (!_packerUtils->ValidatePeFile(_inputFile))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool ResourcePacker::PrepareLoaderStub()
  {
    if (!_packerUtils->PrepareResourceLoaderStub(_inputFile, _peLoader))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool ResourcePacker::RebaseIfNeeded()
  {
    if (!_inputFile.HasRelocationTable() && _inputFile.IntersectsWith(_peLoader))
    {
      ULONGLONG newBase = _inputFile.GetNextImageBase();
      if (!_peLoader.Rebase(newBase))
      {
        SetLastError(Error::ErrorCode::Error_Rebasing);
        return false;
      }
    }
    return true;
  }

  bool ResourcePacker::SavePeLoader(LPWSTR fileName)
  {
    if (!_packerUtils->SavePeFile(fileName, _peLoader))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool ResourcePacker::AppendResourcesToLoader(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    if (!_packerUtils->AppendResources(inputFileName, outputFileName))
    {
      SetLastError(_packerUtils->GetLastError());
      return false;
    }
    return true;
  }

  bool ResourcePacker::CompressAndEmbed(LPWSTR outputFileName)
  {
    Compressor::Compressor compressor;
    BYTE* compressed = nullptr;
    size_t compressedSize = 0;

    if (!compressor.Compress(_inputFile.GetBuffer(), _inputFile.GetBufferSize(), &compressed, &compressedSize))
    {
      SetLastError(Error::ErrorCode::Error_Compressing);
      return false;
    }

    HANDLE updateHandle = BeginUpdateResourceW(outputFileName, FALSE);
    if (updateHandle == nullptr)
    {
      SetLastError(Error::ErrorCode::Error_Appending_Compressed_Data);
      return false;
    }
    if (UpdateResource(updateHandle, RT_RCDATA, MAKEINTRESOURCE(1000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), compressed, (DWORD)compressedSize) == FALSE)
    {
      SetLastError(Error::ErrorCode::Error_Appending_Compressed_Data);
      return false;
    }
    if (EndUpdateResource(updateHandle, FALSE) == FALSE)
    {
      SetLastError(Error::ErrorCode::Error_Appending_Compressed_Data);
      return false;
    }

    compressor.Free(compressed);

    return true;
  }
}