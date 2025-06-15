#include "PELoaderStub.h"
#include "..\ResourceLoader\ResourceLoader.h"
#include "..\Decompressor\Decompressor.h"
#include "..\PELoader\PELoader\PELoader.h"
#include "..\PELoader\PELoader\PEImage.h"
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
    ResourceLoader::ResourceLoader resourceLoader;

    HRSRC resourceHandle = resourceLoader.FindResource(L"FPACK", RT_RCDATA);
    if (resourceHandle != nullptr)
    {
      return LoadFromResource(tlsResolver, &resourceLoader);
    }
    else
    {
      return LoadFromSection(tlsResolver);
    }
  }

  PELoader::PEImage* PELoaderStub::LoadFromResource(PELoader::TlsResolver* tlsResolver, ResourceLoader::ResourceLoader* resourceLoader)
  {
    // Load packed target from resource
    DWORD targetSize = 0;
    BYTE* targetBuffer = resourceLoader->LoadResource(L"FPACK", RT_RCDATA, targetSize);
    if (targetBuffer == nullptr || targetSize == 0) return 0;

    // Decompress target
    size_t decompressedSize = 0;
    BYTE* decompressed = NULL;
    Decompressor::Decompressor decompressor;
    decompressor.Decompress(targetBuffer, targetSize, &decompressed, &decompressedSize);
    resourceLoader->Free(targetBuffer);

    // Load target
    PELoader::PELoader peLoader;
    LPVOID imageBase = peLoader.LoadPE(tlsResolver, (LPVOID)decompressed, true);
    decompressor.Free(decompressed);

    return new PELoader::PEImage(imageBase);
  }

  PELoader::PEImage* PELoaderStub::LoadFromSection(PELoader::TlsResolver* tlsResolver)
  {
    // Load packed target from last section (.reloc)
    PELoader::PEImage self(GetModuleHandle(nullptr));
    WORD lastSectionIndex = self.NT_HEADERS()->FileHeader.NumberOfSections - 1;
    PIMAGE_SECTION_HEADER lastSectionHeader = &self.SECTION_HEADER()[lastSectionIndex];

    BYTE* targetBuffer = (BYTE*)self.GetImageBase() + lastSectionHeader->VirtualAddress;
    DWORD targetSize = lastSectionHeader->SizeOfRawData;
    DWORD oldProtect = 0;
    VirtualProtect(targetBuffer, lastSectionHeader->Misc.VirtualSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Decompress target
    size_t decompressedSize = 0;
    BYTE* decompressed = NULL;
    Decompressor::Decompressor decompressor;
    decompressor.Decompress(targetBuffer, targetSize, &decompressed, &decompressedSize);
    memset(targetBuffer, 0, lastSectionHeader->Misc.VirtualSize);

    // Load target
    PELoader::PELoader peLoader;
    LPVOID imageBase = peLoader.LoadPE(tlsResolver, (LPVOID)decompressed, false);
    decompressor.Free(decompressed);

    return new PELoader::PEImage(imageBase);
  }
}