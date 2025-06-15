#include "CommandLine.h"
#include "Result.h"

namespace CommandLine
{
  CommandLine::CommandLine()
  {
  }

  CommandLine::~CommandLine()
  {
  }

  Result CommandLine::Parse()
  {
    if (!LoadArguments()) return _result;

    if (!ValidateArgumentCount()) return _result;
    SetFileNames();

    if (!AreFileNamesDifferent()) return _result;
    if (!ParseOption()) return _result;

    _result.IsValid = true;
    return _result;
  }

  bool CommandLine::LoadArguments()
  {
    LPWSTR args = ::GetCommandLine();
    _argumentVector = ::CommandLineToArgvW(args, &_argumentCount);
    return (_argumentVector != nullptr);
  }

  bool CommandLine::ValidateArgumentCount()
  {
    if (_argumentCount < 3 || _argumentCount > 4)
    {
      SetErrorMessage(L"Invalid number of arguments.");
      return false;
    }
    return true;
  }

  bool CommandLine::SetFileNames()
  {
    _result.InputFileName = _argumentVector[1];
    _result.OutputFileName = _argumentVector[2];
    return true;
  }

  bool CommandLine::AreFileNamesDifferent()
  {
    if (wcscmp(_result.InputFileName, _result.OutputFileName) == 0)
    {
      SetErrorMessage(L"Input and output files must be different.");
      return false;
    }
    return true;
  }

  bool CommandLine::ParseOption()
  {
    if (_argumentCount == 4)
    {
      LPWSTR option = _argumentVector[3];
      if (wcscmp(option, L"-r") == 0 || wcscmp(option, L"--resource") == 0)
      {
        _result.Option = OPTION::RESOURCE;
      }
      else if (wcscmp(option, L"-s") == 0 || wcscmp(option, L"--section") == 0)
      {
        _result.Option = OPTION::SECTION;
      }
      else
      {
        SetErrorMessage(L"Unknown option.");
        return false;
      }
    }
    return true;
  }

  void CommandLine::SetErrorMessage(const WCHAR* message)
  {
    size_t length = wcslen(message);
    size_t count = (length < MAX_LEN - 1) ? length : MAX_LEN - 1;
    memcpy(_result.ErrorMessage, message, count * sizeof(WCHAR));
    _result.ErrorMessage[count] = L'\0';
  }
}