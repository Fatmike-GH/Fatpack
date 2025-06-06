#include "Compressor.h"
#include "lzma\wrapper\easylzmawrapper.h"

namespace Compressor
{
  Compressor::Compressor()
  {
  }

  Compressor::~Compressor()
  {
  }

  bool Compressor::Compress(BYTE* source, size_t sourceLength, BYTE** compressed, size_t* compressedSize)
  {
    int result = simpleCompress(ELZMA_lzma, source, sourceLength, compressed, compressedSize);
    return (result == ELZMA_E_OK && *compressed != nullptr && *compressedSize != 0);
  }

  void Compressor::Free(BYTE* compressed)
  {
    if (compressed == nullptr) return;
    free(compressed);
  }
}