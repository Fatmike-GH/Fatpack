#include "BinaryFileWriter.h"

namespace BinaryFileWriter
{
  BinaryFileWriter::BinaryFileWriter()
  {
  }

  BinaryFileWriter::~BinaryFileWriter()
  {
  }

  bool BinaryFileWriter::WriteFile(LPCWSTR fileName, BYTE* buffer, DWORD bufferSize)
  {
    if (fileName == nullptr || buffer == nullptr || bufferSize == 0) return false;

    DWORD bytesWritten = 0;
    HANDLE outputFileHandle = ::CreateFileW(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (outputFileHandle == INVALID_HANDLE_VALUE) return false;

    bool result = ::WriteFile(outputFileHandle, buffer, bufferSize, &bytesWritten, nullptr);
    CloseHandle(outputFileHandle);

    return (result && bytesWritten > 0);
  }
}


