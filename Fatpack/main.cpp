#include "CommandLine\CommandLine.h"
#include "Console\Console.h"
#include "Packer\PackerUtils.h"
#include "Packer\ResourcePacker.h"
#include "Packer\SectionPacker.h"

int main()
{
  LPWSTR inputFileName = nullptr;
  LPWSTR outputFileName = nullptr;
  CommandLine::Option option = CommandLine::Option::Resource;

  Console::Console console;
  CommandLine::CommandLine commandLine;
  auto result = commandLine.Parse();
  if (!result.IsValid)
  {
    console.WriteError(result.ErrorMessage);
    console.ShowHelp();
    return 0;
  }

  inputFileName = result.InputFileName;
  outputFileName = result.OutputFileName;
  option = result.Option;

  bool success = false;
  Packer::PackerUtils packerUtils;
  if (option == CommandLine::Option::Resource)
  {
    console.WriteLine(L"Using option -r, --resource");
    Packer::ResourcePacker packer(&packerUtils);
    success = packer.Pack(inputFileName, outputFileName);
  }
  else
  {
    console.WriteLine(L"Using option -s, --section");
    Packer::SectionPacker packer(&packerUtils);
    success = packer.Pack(inputFileName, outputFileName);
  }

  if (success)
    console.WriteLine(L"Packing finished.");
  else
    console.WriteError(L"Packing failed.");

  return success ? 0 : 1;
}