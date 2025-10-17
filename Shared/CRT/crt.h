#pragma once
#include <Windows.h>
#include <intrin.h>

void* __cdecl memset(void* _Dst, int _Val, size_t _Size);
#pragma intrinsic(memset)

void* __cdecl memcpy(void* _Dst, void const* _Src, size_t _Size);
#pragma intrinsic(memcpy)

void* __cdecl memmove(void* dest, const void* src, size_t n);
#pragma intrinsic(memmove)

_Check_return_
int __cdecl strncmp(
  _In_reads_or_z_(_MaxCount) char const* _Str1,
  _In_reads_or_z_(_MaxCount) char const* _Str2,
  _In_                       size_t      _MaxCount
);

_Check_return_
_ACRTIMP size_t __cdecl wcslen(
  _In_z_ wchar_t const* _String
);
#pragma intrinsic(wcslen)

_Check_return_
_ACRTIMP int __cdecl wcscmp(
  _In_z_ wchar_t const* _String1,
  _In_z_ wchar_t const* _String2
);
#pragma intrinsic(wcscmp)

_Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(_Size)
_CRTALLOCATOR _CRT_JIT_INTRINSIC _CRTRESTRICT _CRT_HYBRIDPATCHABLE
void* __cdecl malloc(
  _In_ _CRT_GUARDOVERFLOW size_t _Size
);

_CRT_HYBRIDPATCHABLE
void __cdecl free(
  _Pre_maybenull_ _Post_invalid_ void* _Block
);

_Success_(return != 0) _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(_Size)
_CRTALLOCATOR _CRTRESTRICT _CRT_HYBRIDPATCHABLE
void* __cdecl realloc(
  _Pre_maybenull_ _Post_invalid_ void* _Block,
  _In_ _CRT_GUARDOVERFLOW        size_t _Size
);

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr, size_t size);

