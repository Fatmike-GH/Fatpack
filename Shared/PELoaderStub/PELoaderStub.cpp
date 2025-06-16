#include "PELoaderStub.h"
#include "..\ResourceLoader\ResourceLoader.h"
#include "..\Decompressor\Decompressor.h"
#include "..\PELoader\PELoader\PELoader.h"
#include "..\PELoader\PELoader\PEImage.h"
#include "..\PELoader\PELoader\PEFile.h"
#include "..\PELoader\PELoader\TlsResolver.h"

namespace PELoaderStub
{
  PELoaderStub::PELoaderStub()
  {
  }

  PELoaderStub::~PELoaderStub()
  {
  }

  PELoader::PEImage* PELoaderStub::Load(PELoader::TlsResolver* tlsResolver)
  {
    return IsResourceAvailable() ? LoadFromResource(tlsResolver) : LoadFromSection(tlsResolver);
  }

  PELoader::PEImage* PELoaderStub::LoadFromResource(PELoader::TlsResolver* tlsResolver)
  {
    DWORD compressedSize = 0;
    BYTE* compressedData = GetCompressedDataFromResource(compressedSize);
    if (!compressedData || compressedSize == 0) return nullptr;

    size_t decompressedSize = 0;
    BYTE* decompressedData = Decompress(compressedData, compressedSize, decompressedSize);
    delete[] compressedData;

    return CreatePEImageFromMemory(decompressedData, tlsResolver);
  }

  PELoader::PEImage* PELoaderStub::LoadFromSection(PELoader::TlsResolver* tlsResolver)
  {
    DWORD rawSize = 0, virtualSize = 0;
    BYTE* sectionBase = GetLastSection(rawSize, virtualSize);
    if (!sectionBase || rawSize == 0) return nullptr;

    size_t decompressedSize = 0;
    BYTE* decompressedData = Decompress(sectionBase, rawSize, decompressedSize);
    SecureZeroMemory(sectionBase, virtualSize);

    return CreatePEImageFromMemory(decompressedData, tlsResolver, sectionBase);
  }

  bool PELoaderStub::IsResourceAvailable()
  {
    ResourceLoader::ResourceLoader resourceLoader;
    return resourceLoader.FindResource(L"FPACK", RT_RCDATA) != nullptr;
  }

  BYTE* PELoaderStub::GetCompressedDataFromResource(DWORD& compressedSize)
  {
    ResourceLoader::ResourceLoader resourceLoader;
    return resourceLoader.LoadResource(L"FPACK", RT_RCDATA, compressedSize);
  }

  BYTE* PELoaderStub::GetLastSection(DWORD& rawSize, DWORD& virtualSize)
  {
    PELoader::PEImage self(GetModuleHandle(nullptr));
    WORD lastSectionIndex = self.GetNumberOfSections() - 1;
    PIMAGE_SECTION_HEADER lastSectionHeader = self.GetSectionHeader(lastSectionIndex);

    BYTE* sectionStart = (BYTE*)self.GetImageBase() + lastSectionHeader->VirtualAddress;
    rawSize = lastSectionHeader->SizeOfRawData;
    virtualSize = lastSectionHeader->Misc.VirtualSize;

    DWORD oldProtect = 0;
    VirtualProtect(sectionStart, virtualSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    return sectionStart;
  }

  BYTE* PELoaderStub::Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize)
  {
    BYTE* decompressedData = nullptr;
    Decompressor::Decompressor decompressor;
    decompressor.Decompress(compressedData, compressedSize, &decompressedData, &decompressedSize);
    return decompressedData;
  }

  PELoader::PEImage* PELoaderStub::CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase /*= nullptr*/)
  {
    if (!peFileBuffer) return nullptr;

    PELoader::PELoader peLoader;
    PELoader::PEFile peFile(peFileBuffer);

    allocatedImageBase = peLoader.LoadPE(&peFile, tlsResolver, allocatedImageBase);

    delete[] peFileBuffer;
    return new PELoader::PEImage(allocatedImageBase);
  }
}