#pragma once
#include <Windows.h>
#include "..\Error\Error.h"

namespace Console
{
  class Console
  {
  public:
    Console();
    ~Console();

    void WriteLine(const wchar_t* text);
    void WriteError(Error::ErrorCode errorCode);
    void WriteError(const wchar_t* text);
    void ShowHelp();

  private:
    const wchar_t* GetErrorString(Error::ErrorCode code);

  private:
    HANDLE _consoleHandle;
  };
}