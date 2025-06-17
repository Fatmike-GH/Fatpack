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
  class ResourceLoaderStub
  {
  public:
    ResourceLoaderStub();
    ~ResourceLoaderStub();

    PELoader::PEImage* Load(PELoader::TlsResolver* tlsResolver);

  private:
    PELoader::PEImage* LoadFromResource(PELoader::TlsResolver* tlsResolver);
    BYTE* GetCompressedDataFromResource(DWORD& compressedSize);
    BYTE* Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize);
    PELoader::PEImage* CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase = nullptr);
  };
}