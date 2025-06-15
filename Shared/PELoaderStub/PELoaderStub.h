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
    PELoader::PEImage* LoadFromResource(PELoader::TlsResolver* tlsResolver, ResourceLoader::ResourceLoader* resourceLoader);
    PELoader::PEImage* LoadFromSection(PELoader::TlsResolver* tlsResolver);
  };
}