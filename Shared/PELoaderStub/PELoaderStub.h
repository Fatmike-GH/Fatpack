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
  class PELoaderStub
  {
  public:
    PELoaderStub();
    ~PELoaderStub();

    PELoader::PEImage* Load(PELoader::TlsResolver* tlsResolver);

  private:

    bool IsResourceAvailable();

    PELoader::PEImage* LoadFromResource(PELoader::TlsResolver* tlsResolver);
    PELoader::PEImage* LoadFromSection(PELoader::TlsResolver* tlsResolver);

    BYTE* GetCompressedDataFromResource(DWORD& compressedSize);
    BYTE* GetLastSection(DWORD& rawSize, DWORD& virtualSize);

    BYTE* Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize);
    PELoader::PEImage* CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase = nullptr);
  };
}