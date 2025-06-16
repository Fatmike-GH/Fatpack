#include "Decompressor.h"
#include "TinyLzmaDecompress.h"
#include "..\..\Shared\CRT\crt.h"

namespace Decompressor
{
  Decompressor::Decompressor()
  {
  }

  Decompressor::~Decompressor()
  {
  }

  bool Decompressor::Decompress(BYTE* source, size_t sourceLength, BYTE** decompressed, size_t* uncompressedSize)
  {
    return (tinyLzmaDecompress(source, sourceLength, decompressed, uncompressedSize) == 0);
  }
}