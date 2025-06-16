#pragma once
#include <Windows.h>
#include "Option.h"
#include "..\Error\Error.h"

namespace CommandLine
{
  class CommandLine
  {
  public:
    CommandLine();
    ~CommandLine();

    bool Parse(LPWSTR& inputFileName, LPWSTR& outputFileName, Option& option);
    Error::ErrorCode GetLastError() { return _lastError; }   

  private:
    bool LoadArguments();
    bool ValidateArgumentCount();
    bool SetFileNames(LPWSTR& inputFileName, LPWSTR& outputFileName);
    bool AreFileNamesDifferent(LPWSTR inputFileName, LPWSTR outputFileName);
    bool ParseOption(Option& option);
    void SetLastError(Error::ErrorCode error) { _lastError = error; }

  private:
    int _argumentCount = 0;
    LPWSTR* _argumentVector = nullptr;
    Error::ErrorCode _lastError;

    const DWORD MAX_LEN = 256;
  };
}