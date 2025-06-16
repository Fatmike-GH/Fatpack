#pragma once
#include <Windows.h>

namespace Decompressor
{
  class Decompressor
  {
  public:
    Decompressor();
    ~Decompressor();

    bool Decompress(BYTE* source, size_t sourceLength, BYTE** decompressed, size_t* compressedSize);
  };
}