#include "Console.h"

namespace Console
{
  Console::Console()
  {
  }

  Console::~Console()
  {
  }

  void Console::WriteLine(const wchar_t* text)
  {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandle == INVALID_HANDLE_VALUE) return;

    DWORD written = 0;
    WriteConsoleW(consoleHandle, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(consoleHandle, L"\n", 1, &written, NULL);
  }

  void Console::WriteError(const wchar_t* text)
  {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandle == INVALID_HANDLE_VALUE) return;

    DWORD written = 0;
    const wchar_t* error = L"Error : ";
    WriteConsoleW(consoleHandle, error, (DWORD)wcslen(error), &written, NULL);
    WriteConsoleW(consoleHandle, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(consoleHandle, L"\n", 1, &written, NULL);
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
}