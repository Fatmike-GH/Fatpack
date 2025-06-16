#include "CommandLine.h"

namespace CommandLine
{
  CommandLine::CommandLine()
  {
    _lastError = Error::ErrorCode::Ok;
  }

  CommandLine::~CommandLine()
  {
  }

  bool CommandLine::Parse(LPWSTR& inputFileName, LPWSTR& outputFileName, Option& option)
  {
    SetLastError(Error::ErrorCode::Ok);

    return LoadArguments() &&
           ValidateArgumentCount() &&
           SetFileNames(inputFileName, outputFileName) &&
           AreFileNamesDifferent(inputFileName, outputFileName) &&
           ParseOption(option);
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
      SetLastError(Error::ErrorCode::Error_Argument_Count);
      return false;
    }
    return true;
  }

  bool CommandLine::SetFileNames(LPWSTR& inputFileName, LPWSTR& outputFileName)
  {
    inputFileName = _argumentVector[1];
    outputFileName = _argumentVector[2];
    return true;
  }

  bool CommandLine::AreFileNamesDifferent(LPWSTR inputFileName, LPWSTR outputFileName)
  {
    if (wcscmp(inputFileName, outputFileName) == 0)
    {
      SetLastError(Error::ErrorCode::Error_Equal_Filenames);
      return false;
    }
    return true;
  }

  bool CommandLine::ParseOption(Option& option)
  {
    option = Option::Resource;
    if (_argumentCount == 4)
    {
      LPWSTR input = _argumentVector[3];
      if (wcscmp(input, L"-r") == 0 || wcscmp(input, L"--resource") == 0)
      {
        option = Option::Resource;
      }
      else if (wcscmp(input, L"-s") == 0 || wcscmp(input, L"--section") == 0)
      {
        option = Option::Section;
      }
      else
      {
        SetLastError(Error::ErrorCode::Error_Unknown_Option);
        return false;
      }
    }
    return true;
  }
}