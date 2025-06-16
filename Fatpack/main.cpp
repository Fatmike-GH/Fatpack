#include "CommandLine\CommandLine.h"
#include "Console\Console.h"
#include "Error\Error.h"
#include "Packer\PackerUtils.h"
#include "Packer\ResourcePacker.h"
#include "Packer\SectionPacker.h"

int main()
{
  Console::Console console;
  CommandLine::CommandLine commandLine;

  LPWSTR inputFileName = nullptr;
  LPWSTR outputFileName = nullptr;
  CommandLine::Option option = CommandLine::Option::Resource;

  if (!commandLine.Parse(inputFileName, outputFileName, option))
  {
    console.WriteError(commandLine.GetLastError());
    console.ShowHelp();
    return 0;
  }

  bool success = false;
  Error::ErrorCode error = Error::ErrorCode::Ok;

  Packer::PackerUtils packerUtils;
  if (option == CommandLine::Option::Resource)
  {
    console.WriteLine(L"Using option -r, --resource");
    Packer::ResourcePacker packer(&packerUtils);
    success = packer.Pack(inputFileName, outputFileName);
    error = packer.GetLastError();
  }
  else
  {
    console.WriteLine(L"Using option -s, --section");
    Packer::SectionPacker packer(&packerUtils);
    success = packer.Pack(inputFileName, outputFileName);
    error = packer.GetLastError();
  }

  if (success)
  {
    console.WriteLine(L"Packing finished.");
  }    
  else
  {
    console.WriteError(L"Packing failed.");
    console.WriteError(error);
  }

  return 0;
}