#pragma once
#include <windows.h>

namespace PELoader
{
  class PEImage;
  class TlsResolver;
}

namespace ResourceLoader
{
  class ResourceLoader;
}

namespace PELoaderStub
{
  class SectionLoaderStub
  {
  public:
    SectionLoaderStub();
    ~SectionLoaderStub();

    PELoader::PEImage* Load(PELoader::TlsResolver* tlsResolver);

  private:
    PELoader::PEImage* LoadFromSection(PELoader::TlsResolver* tlsResolver);
    BYTE* GetLastSection(DWORD& rawSize, DWORD& virtualSize);
    BYTE* Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize);
    PELoader::PEImage* CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase = nullptr);
  };
}