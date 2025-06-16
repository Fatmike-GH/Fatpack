#pragma once
#include <Windows.h>

namespace Compressor
{
  class Compressor
  {
  public:
    Compressor();
    ~Compressor();

    bool Compress(BYTE* source, size_t sourceLength, BYTE** compressed, size_t* compressedSize);
    void Free(BYTE* compressed);
  };
}