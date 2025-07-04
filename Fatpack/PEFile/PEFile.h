#pragma once
#include <windows.h>

namespace PEFile
{
  class PEFile
  {
  public:
    PEFile();
    ~PEFile();

    bool LoadFromBuffer(BYTE* fileBuffer, DWORD size);
    bool Resize(DWORD size);

    DWORD GetBufferSize() { return _bufferSize; }
    BYTE* GetBuffer() { return _buffer; }

    const PIMAGE_DOS_HEADER DOS_HEADER() { return _PIMAGE_DOS_HEADER; }
    const PIMAGE_NT_HEADERS NT_HEADERS() { return _PIMAGE_NT_HEADERS; }
    const PIMAGE_SECTION_HEADER SECTION_HEADER() { return _PIMAGE_SECTION_HEADER; }

    uintptr_t GetImageBase();
    DWORD GetSizeOfImage();

    bool IsPEFile();    
    bool Isx64();
    bool IsNative();
    bool IsConsole();


    WORD GetSectionCount();
    PIMAGE_SECTION_HEADER GetSectionHeader(WORD index);

    static uintptr_t AlignImageBase(uintptr_t imageBase);
    DWORD AlignSection(DWORD value);
    DWORD AlignFile(DWORD value);

    bool IntersectsWith(PEFile& other);
    bool HasRelocationTable();
    ULONGLONG GetNextImageBase();
    bool Rebase(ULONGLONG newImageBase);

  private:

    bool ApplyRelocations(ULONGLONG imageBase);
    void SetImageBase(ULONGLONG imageBase);
    void StripRelocations();

    DWORD Rva2RawOffset(DWORD rva);
    DWORD64 GetRawAddress(DWORD rawOffset);
    
    void DeleteBuffer();

  private:
    BYTE* _buffer;
    DWORD _bufferSize;

    PIMAGE_DOS_HEADER _PIMAGE_DOS_HEADER;
    PIMAGE_NT_HEADERS _PIMAGE_NT_HEADERS;
    PIMAGE_SECTION_HEADER _PIMAGE_SECTION_HEADER;
  };
}