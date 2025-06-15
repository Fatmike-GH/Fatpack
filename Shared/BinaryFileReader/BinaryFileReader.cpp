#include "BinaryFileReader.h"

namespace BinaryFileReader
{
  BinaryFileReader::BinaryFileReader(const wchar_t* fileName)
  {
    _buffer = nullptr;
    _bufferSize = 0;

    HANDLE fileHandle = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
      Cleanup();
      return;
    }

    _bufferSize = GetFileSize(fileHandle, NULL);
    if (_bufferSize == INVALID_FILE_SIZE)
    {
      Cleanup();
      CloseHandle(fileHandle);
      return;
    }

    _buffer = new BYTE[_bufferSize];

    DWORD bytesRead = 0;
    BOOL result = ReadFile(fileHandle, _buffer, _bufferSize, &bytesRead, NULL);
    CloseHandle(fileHandle);

    if (result == FALSE)
    {
      Cleanup();
    }

    return;
  }

  BinaryFileReader::~BinaryFileReader()
  {
    Cleanup();
  }

  void BinaryFileReader::Cleanup()
  {
    if (_buffer != nullptr)
    {
      delete[] _buffer;
      _buffer = nullptr;
    }
    _bufferSize = 0;
  }
}


