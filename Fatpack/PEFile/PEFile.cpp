#include "PEFile.h"

namespace PEFile
{
  PEFile::PEFile()
  {
    _buffer = nullptr;
    _bufferSize = 0;
    _PIMAGE_DOS_HEADER = nullptr;
    _PIMAGE_NT_HEADERS = nullptr;
    _PIMAGE_SECTION_HEADER = nullptr;
  }

  PEFile::~PEFile()
  {
    DeleteBuffer();
  }

  bool PEFile::LoadFromBuffer(BYTE* fileBuffer, DWORD size)
  {
    if (fileBuffer == nullptr || size == 0) return false;

    DeleteBuffer();
    _bufferSize = size;
    _buffer = new BYTE[_bufferSize];
    memcpy(_buffer, fileBuffer, _bufferSize);

    _PIMAGE_DOS_HEADER = (PIMAGE_DOS_HEADER)_buffer;
    _PIMAGE_NT_HEADERS = (PIMAGE_NT_HEADERS)(_buffer + _PIMAGE_DOS_HEADER->e_lfanew);
    _PIMAGE_SECTION_HEADER = IMAGE_FIRST_SECTION(_PIMAGE_NT_HEADERS);

    return true;
  }

  ULONGLONG PEFile::GetImageBase()
  {
    if (_buffer == nullptr || _bufferSize == 0) return 0;
    return _PIMAGE_NT_HEADERS->OptionalHeader.ImageBase;
  }

  DWORD PEFile::GetSizeOfImage()
  {
    if (_buffer == nullptr || _bufferSize == 0) return 0;
    return _PIMAGE_NT_HEADERS->OptionalHeader.SizeOfImage;
  }

  bool PEFile::IsPEFile()
  {
    if (_buffer == nullptr || _bufferSize == 0) return false;
    return (_PIMAGE_DOS_HEADER->e_magic == IMAGE_DOS_SIGNATURE && _PIMAGE_NT_HEADERS->Signature == IMAGE_NT_SIGNATURE);
  }

  bool PEFile::Isx64()
  {
    if (_buffer == nullptr || _bufferSize == 0) return false;
    return (_PIMAGE_NT_HEADERS->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
  }

  bool PEFile::IsNative()
  {
    if (_buffer == nullptr || _bufferSize == 0) return false;
    if (!Isx64()) return false;
    PIMAGE_DATA_DIRECTORY comDirectory = &_PIMAGE_NT_HEADERS->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
    return (comDirectory->VirtualAddress == 0 && comDirectory->Size == 0);
  }

  bool PEFile::IsConsole()
  {
    if (_buffer == nullptr || _bufferSize == 0) return false;
    if (!Isx64()) return false;
    return (_PIMAGE_NT_HEADERS->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);
  }

  bool PEFile::IntersectsWith(PEFile& other)
  {
    auto imageBase = GetImageBase();
    auto imageEnd = imageBase + GetSizeOfImage();
    auto otherImageBase = other.GetImageBase();
    auto otherImageEnd = otherImageBase + other.GetSizeOfImage();

    if (imageBase == otherImageBase) return true;
    if (otherImageBase >= imageBase && otherImageBase <= imageEnd) return true;
    if (otherImageBase <= imageBase && otherImageEnd >= imageBase) return true;

    return false;
  }

  bool PEFile::HasRelocationTable()
  {
    if (_buffer == nullptr || _bufferSize == 0) return false;

    DWORD size = _PIMAGE_NT_HEADERS->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    DWORD virtualAddress = _PIMAGE_NT_HEADERS->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    return (size != 0 && virtualAddress != 0);
  }

  ULONGLONG PEFile::GetNextImageBase()
  {
    if (_buffer == nullptr || _bufferSize == 0) return 0;

    ULONGLONG result = 0;
    const DWORD imageBaseAlignment = 0x00010000; // ImageBase must be a multiple of 64 K

    ULONGLONG endOfImage = GetImageBase() + GetSizeOfImage();
    if (endOfImage % imageBaseAlignment == 0)
    {
      result = endOfImage;
    }
    else
    {
      result = imageBaseAlignment * ((endOfImage / imageBaseAlignment) + 1);
    }
    return result;
  }

  bool PEFile::Rebase(ULONGLONG newImageBase)
  {
    if (!ApplyRelocations(newImageBase)) return false;
    SetImageBase(newImageBase);
    StripRelocations();
    return true;
  }

  bool PEFile::ApplyRelocations(ULONGLONG imageBase)
  {
    if (_buffer == nullptr || _bufferSize == 0 || !HasRelocationTable() || imageBase == GetImageBase()) return false;

    ULONGLONG baseDelta = (ULONGLONG)(imageBase - GetImageBase());
    if (baseDelta == 0) return false;

    PIMAGE_DATA_DIRECTORY relocDir = &NT_HEADERS()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (relocDir->Size == 0) return false;

    PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION)(GetRawAddress(Rva2RawOffset(relocDir->VirtualAddress)));
    while (reloc->VirtualAddress && reloc->SizeOfBlock)
    {
      int numEntries = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
      PWORD relocEntries = (PWORD)((BYTE*)reloc + sizeof(IMAGE_BASE_RELOCATION));

      for (int i = 0; i < numEntries; i++)
      {
        WORD type = relocEntries[i] >> 12;
        WORD offset = relocEntries[i] & 0xFFF;

        switch (type)
        {
          case IMAGE_REL_BASED_ABSOLUTE:
            break;

          case IMAGE_REL_BASED_DIR64:
          {
            ULONGLONG* patchAddr = (ULONGLONG*)(GetRawAddress(Rva2RawOffset(reloc->VirtualAddress)) + offset);
            *patchAddr += baseDelta;
            break;
          }
          default:
            break;
        }
      }

      reloc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc + reloc->SizeOfBlock);
    }

    return true;
  }

  void PEFile::SetImageBase(ULONGLONG imageBase)
  {
    if (_buffer == nullptr || _bufferSize == 0) return;

    NT_HEADERS()->OptionalHeader.ImageBase = imageBase;
  }

  void PEFile::StripRelocations()
  {
    if (_buffer == nullptr || _bufferSize == 0 || !HasRelocationTable()) return;

    DOS_HEADER()->e_crlc = 0;

    NT_HEADERS()->FileHeader.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;
    NT_HEADERS()->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
    NT_HEADERS()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
    NT_HEADERS()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;

    // To do (optional): Remove section from pe file if its the last section to save some extra bytes
  }

  DWORD PEFile::Rva2RawOffset(DWORD rva)
  {
    WORD sectionCount = NT_HEADERS()->FileHeader.NumberOfSections;
    PIMAGE_SECTION_HEADER sectionHeader = SECTION_HEADER();
    for (int i = 0; i < sectionCount; i++, sectionHeader++)
    {
      DWORD startOfSection = sectionHeader->VirtualAddress;
      DWORD endOfSection = sectionHeader->VirtualAddress + sectionHeader->SizeOfRawData;
      if (rva >= startOfSection && rva <= endOfSection)
      {
        DWORD relativeOffset = rva - startOfSection;
        DWORD rawOffset = sectionHeader->PointerToRawData + relativeOffset;
        return rawOffset;
      }
    }
    return -1;
  }

  DWORD64 PEFile::GetRawAddress(DWORD rawOffset)
  {
    return (DWORD64)((BYTE*)DOS_HEADER() + rawOffset);
  }

  void PEFile::DeleteBuffer()
  {
    if (_buffer != nullptr)
    {
      delete[] _buffer;
      _buffer = nullptr;
    }
  }
}