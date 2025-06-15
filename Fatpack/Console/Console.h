#pragma once
#include <Windows.h>

namespace Console
{
  class Console
  {
  public:
    Console();
    ~Console();

    void WriteLine(const wchar_t* text);
    void WriteError(const wchar_t* text);
    void ShowHelp();
  };
}