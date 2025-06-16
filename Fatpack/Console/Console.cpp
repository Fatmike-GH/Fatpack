#include "Console.h"

namespace Console
{
  Console::Console()
  {
    _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  }

  Console::~Console()
  {
  }

  void Console::WriteLine(const wchar_t* text)
  {
    DWORD written = 0;
    WriteConsoleW(_consoleHandle, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(_consoleHandle, L"\n", 1, &written, NULL);
  }

  void Console::WriteError(Error::ErrorCode errorCode)
  {
    WriteError(GetErrorString(errorCode));
  }

  void Console::ShowHelp()
  {
    WriteLine(L"\n..::[Fatmike 2025]::..\n");
    WriteLine(L"Version: Fatpack v1.5.1");
    WriteLine(L"Usage:\t fatpack.exe inputfile.exe outputfile.exe [OPTIONS]\n");
    WriteLine(L"[OPTIONS]");
    WriteLine(L"-r, --resource\t Packs inputfile.exe as resource (DEFAULT)");
    WriteLine(L"-s, --section\t Packs inputfile.exe as section (EXPERIMENTAL)");
    WriteLine(L"\nNOTES");
    WriteLine(L"-s, --section is the preferred option as it requires less memory at runtime.\nHowever, it may cause issues with certain targets in specific cases.");
  }

  void Console::WriteError(const wchar_t* text)
  {
    DWORD written = 0;
    const wchar_t* error = L"Error : ";
    WriteConsoleW(_consoleHandle, error, (DWORD)wcslen(error), &written, NULL);
    WriteConsoleW(_consoleHandle, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(_consoleHandle, L"\n", 1, &written, NULL);
  }

  const wchar_t* Console::GetErrorString(Error::ErrorCode code)
  {
    switch (code)
    {
      case Error::ErrorCode::Ok: return L"Success.";
      case Error::ErrorCode::Error_Argument_Count: return L"Invalid number of arguments.";
      case Error::ErrorCode::Error_Equal_Filenames: return L"Input and output files must be different.";
      case Error::ErrorCode::Error_Unknown_Option: return L"Unknown option.";
      case Error::ErrorCode::Error_Read_File: return L"Failed to read file.";
      case Error::ErrorCode::Error_Load_Pe_From_Buffer: return L"Failed to load PE file from buffer.";
      case Error::ErrorCode::Error_Pe_Not_Valid: return L"Input file is not a valid PE file.";
      case Error::ErrorCode::Error_Pe_Not_x64: return L"Input file is not a x64 PE file.";
      case Error::ErrorCode::Error_Pe_Not_Native: return L"Input file is not a native PE file.";
      case Error::ErrorCode::Error_Loading_Loader_Stub: return L"Loading loader stub failed.";
      case Error::ErrorCode::Error_Saving_Pe: return L"Failed to save PE file.";
      case Error::ErrorCode::Error_Adding_Icon: return L"Adding icon failed.";
      case Error::ErrorCode::Error_Adding_Manifest: return L"Adding manifest failed.";
      case Error::ErrorCode::Error_Rebasing: return L"Rebasing failed.";
      case Error::ErrorCode::Error_Compressing: return L"Compressing failed.";
      case Error::ErrorCode::Error_Appending_Compressed_Data: return L"Appending compressed data failed.";
      default: return L"Unknown error code.";
    }
  }
}