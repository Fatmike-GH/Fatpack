#include "ResourceLoaderStub.h"
#include "..\..\Shared\ResourceLoader\ResourceLoader.h"
#include "..\..\Shared\Decompressor\Decompressor.h"
#include "..\..\Shared\PELoader\PELoader\PELoader.h"
#include "..\..\Shared\PELoader\PELoader\PEImage.h"
#include "..\..\Shared\PELoader\PELoader\PEFile.h"
#include "..\..\Shared\ApiCaller\ApiCaller.h"

namespace PELoaderStub
{
  ResourceLoaderStub::ResourceLoaderStub()
  {
  }

  ResourceLoaderStub::~ResourceLoaderStub()
  {
  }

  PELoader::PEImage* ResourceLoaderStub::Load(PELoader::TlsResolver* tlsResolver)
  {
    return LoadFromResource(tlsResolver);
  }

  PELoader::PEImage* ResourceLoaderStub::LoadFromResource(PELoader::TlsResolver* tlsResolver)
  {
    DWORD compressedSize = 0;
    BYTE* compressedData = GetCompressedDataFromResource(compressedSize);
    if (!compressedData || compressedSize == 0) return nullptr;

    size_t decompressedSize = 0;
    BYTE* decompressedData = Decompress(compressedData, compressedSize, decompressedSize);
    delete[] compressedData;

    return CreatePEImageFromMemory(decompressedData, tlsResolver);
  }

  BYTE* ResourceLoaderStub::GetCompressedDataFromResource(DWORD& compressedSize)
  {
    ResourceLoader::ResourceLoader resourceLoader;
    return resourceLoader.LoadResource(MAKEINTRESOURCE(1000), RT_RCDATA, compressedSize);
  }

  BYTE* ResourceLoaderStub::Decompress(BYTE* compressedData, DWORD compressedSize, size_t& decompressedSize)
  {
    BYTE* decompressedData = nullptr;
    Decompressor::Decompressor decompressor;
    decompressor.Decompress(compressedData, compressedSize, &decompressedData, &decompressedSize);
    return decompressedData;
  }

  PELoader::PEImage* ResourceLoaderStub::CreatePEImageFromMemory(BYTE* peFileBuffer, PELoader::TlsResolver* tlsResolver, LPVOID allocatedImageBase /*= nullptr*/)
  {
    if (!peFileBuffer) return nullptr;

    PELoader::PELoader peLoader;
    PELoader::PEFile peFile(peFileBuffer);

    allocatedImageBase = peLoader.LoadPE(&peFile, tlsResolver, allocatedImageBase);

    delete[] peFileBuffer;
    return new PELoader::PEImage(allocatedImageBase);
  }
}