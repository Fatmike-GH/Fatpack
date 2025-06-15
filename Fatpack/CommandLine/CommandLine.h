#pragma once
#include <Windows.h>
#include "Result.h"

namespace CommandLine
{
  class CommandLine
  {
  public:
    CommandLine();
    ~CommandLine();

    Result Parse();

  private:
    bool LoadArguments();
    bool ValidateArgumentCount();
    bool SetFileNames();
    bool AreFileNamesDifferent();
    bool ParseOption();
    void SetErrorMessage(const WCHAR* message);

  private:
    Result _result;
    int _argumentCount = 0;
    LPWSTR* _argumentVector = nullptr;    
  };
}