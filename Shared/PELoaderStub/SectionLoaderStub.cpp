#include "SectionLoaderStub.h"
#include "..\..\Shared\Decompressor\Decompressor.h"
#include "..\..\Shared\PELoader\PELoader\PELoader.h"
#include "..\..\Shared\PELoader\PELoader\PEImage.h"
#include "..\..\Shared\PELoader\PELoader\PEFile.h"
#include "..\..\Shared\ApiCaller\ApiCaller.h"

namespace PELoaderStub
{
  SectionLoaderStub::SectionLoaderStub()
  {
  }

  SectionLoaderStub::~SectionLoaderStub()
  {
  }

  PELoader::PEImage* SectionLoaderStub::Load(PELoader::TlsResolver* tlsResolver)
  {
    return LoadFromSection(tlsResolver);
  }

  PELoader::PEImage* SectionLoaderStub::LoadFromSection(PELoader::TlsResolver* tlsResolver)
  {
    DWORD rawSize = 0, virtualSize = 0;
    BYTE* sectionBase = GetLastSection(rawSize, virtualSize);
    if (!sectionBase || rawSize == 0) return nullptr;

    size_t decompressedSize = 0;
    BYTE* decompressedData = Decompress(sectionBase, rawSize, decompressedSize);
    SecureZeroMemory(sectionBase, virtualSize);

    return CreatePEImageFromMemory(decompressedData, tlsResolver, sectionBase);
  }

  BYTE* SectionLoaderStub::GetLastSection(DWORD& rawSize, DWORD& virtualSize)
  {
    PELoader::PEImage self(ApiCaller::ApiCaller::Instance().CallGetModuleHandle(nullptr));
    WORD lastSectionIndex = self.GetNumberOfSections() - 1;
    PIMAGE_SECTION_HEADER lastSectionHeader = self.GetSectionHeader(lastSectionIndex);

    BYTE* sectionStart = (BYTE*)self.GetImageBase() + lastSectionHeader->VirtualAddress;
    rawSize = lastSectionHeader->SizeOfRawData;
    virtualSize = lastSectionHeader->Misc.VirtualSize;

    DWORD oldProtect = 0;
    ApiCaller::ApiCaller::Instance().CallVirtualProtect(sectionStart, virtualSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    return sectionStart;
  }

  BYTE* SectionLoaderStub::Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize)
  {
    BYTE* decompressedData = nullptr;
    Decompressor::Decompressor decompressor;
    decompressor.Decompress(compressedData, compressedSize, &decompressedData, &decompressedSize);
    return decompressedData;
  }

  PELoader::PEImage* SectionLoaderStub::CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase /*= nullptr*/)
  {
    if (!peFileBuffer) return nullptr;

    PELoader::PELoader peLoader;
    PELoader::PEFile peFile(peFileBuffer);

    allocatedImageBase = peLoader.LoadPE(&peFile, tlsResolver, allocatedImageBase);

    delete[] peFileBuffer;
    return new PELoader::PEImage(allocatedImageBase);
  }
}