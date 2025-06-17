#include "crt.h"

#pragma function(memset)
void* memset(void* _Dst, int _Val, size_t _Size)
{
  __stosb(static_cast<unsigned char*>(_Dst),
  static_cast<unsigned char>(_Val), _Size);
  return _Dst;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* _Dst, void const* _Src, size_t _Size)
{
  __movsb(static_cast<unsigned char*>(_Dst),
  static_cast<const unsigned char*>(_Src), _Size);
  return _Dst;
}

#pragma function(memmove)
void* __cdecl memmove(void* dest, const void* src, size_t n)
{
  unsigned char* from = (unsigned char*)src;
  unsigned char* to = (unsigned char*)dest;

  if (from == to || n == 0)
    return dest;
  if (to > from && to - from < (int)n) {
    /* to overlaps with from */
    /*  <from......>         */
    /*         <to........>  */
    /* copy in reverse, to avoid overwriting from */
    size_t i;
    for (i = n - 1; i >= 0; i--)
      to[i] = from[i];
    return dest;
  }
  if (from > to && from - to < (int)n) {
    /* to overlaps with from */
    /*        <from......>   */
    /*  <to........>         */
    /* copy forwards, to avoid overwriting from */
    size_t i;
    for (i = 0; i < n; i++)
      to[i] = from[i];
    return dest;
  }
  memcpy(dest, src, n);
  return dest;
}

_Check_return_
int __cdecl strncmp(
  _In_reads_or_z_(n) char const* s1,
  _In_reads_or_z_(n) char const* s2,
  _In_                       size_t      n
)
{
  register unsigned char u1, u2;

  while (n-- > 0)
  {
    u1 = (unsigned char)*s1++;
    u2 = (unsigned char)*s2++;
    if (u1 != u2)
      return u1 - u2;
    if (u1 == '\0')
      return 0;
  }
  return 0;
}

_Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(size)
_CRTALLOCATOR _CRT_JIT_INTRINSIC _CRTRESTRICT _CRT_HYBRIDPATCHABLE
void* __cdecl malloc(
  _In_ _CRT_GUARDOVERFLOW size_t size
)
{
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

_CRT_HYBRIDPATCHABLE
void __cdecl free(
  _Pre_maybenull_ _Post_invalid_ void* ptr
)
{
  HeapFree(GetProcessHeap(), 0, ptr);
}

_Success_(return != 0) _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(size)
_CRTALLOCATOR _CRTRESTRICT _CRT_HYBRIDPATCHABLE
void* __cdecl realloc(
  _Pre_maybenull_ _Post_invalid_ void* ptr,
  _In_ _CRT_GUARDOVERFLOW        size_t size
)
{
  if (ptr == NULL)
  {
    return malloc(size);
  }
  if (size == 0)
  {
    free(ptr);
    return NULL;
  }

  return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size);
}

void* operator new(size_t size)
{
  if (size == 0) return nullptr;
  return malloc(size);
}

void* operator new[](size_t size)
{
  if (size == 0) return nullptr;
  return malloc(size);
}

void operator delete(void* ptr)
{
  free(ptr);
}

void operator delete[](void* ptr)
{
  free(ptr);
}

void operator delete(void* ptr, size_t size)
{
  free(ptr);
}

void operator delete[](void* ptr, size_t size)
{
  free(ptr);
}