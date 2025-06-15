#include "BinaryFileWriter\BinaryFileWriter.h"
#include "Compessor\Compressor.h"
#include "CommandLine\CommandLine.h"
#include "Console\Console.h"
#include "IconExtractor\IconExtractor.h"
#include "ManifestExtractor\ManifestExtractor.h"
#include "PEFile\PEFile.h"
#include "..\Shared\BinaryFileReader\BinaryFileReader.h"
#include "..\Shared\ResourceLoader\ResourceLoader.h"

bool GetCommandLine(LPWSTR& inputFileName, LPWSTR& outputFileName, CommandLine::OPTION& option);
bool ReadFile(LPWSTR fileName, PEFile::PEFile& peFile);
bool IsInputFileSupported(PEFile::PEFile& inputFile);
bool LoadPELoaderFromResource(PEFile::PEFile& peLoader, bool useConsoleStub);
bool RebasePELoaderIfRequired(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
bool RebasePELoaderToLastSection(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
bool SavePELoader(PEFile::PEFile& peLoader, LPWSTR outputFileName);
bool AppendResourcesToPELoader(LPWSTR inputFileName, LPWSTR outputFileName);
bool CompressAndAppendToPELoaderAsResource(PEFile::PEFile& inputFile, LPWSTR outputFileName);
bool CompressAndAppendToPELoaderIntoLastSection(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);

void PackIntoResource(LPWSTR inputFileName, LPWSTR outputFileName);
void PackIntoLastSection(LPWSTR inputFileName, LPWSTR outputFileName);

int main()
{
  LPWSTR inputFileName = nullptr;
  LPWSTR outputFileName = nullptr;
  CommandLine::OPTION option = CommandLine::OPTION::RESOURCE;
  if (!GetCommandLine(inputFileName, outputFileName, option)) return 0;

  Console::Console console;
  if (option == CommandLine::OPTION::RESOURCE)
  {
    console.WriteLine(L"Using option -r, --resource");
    PackIntoResource(inputFileName, outputFileName);
  }
  else
  {
    console.WriteLine(L"Using option -s, --section");
    PackIntoLastSection(inputFileName, outputFileName);
  }
  return 0;
}

void PackIntoResource(LPWSTR inputFileName, LPWSTR outputFileName)
{
  PEFile::PEFile inputFile;
  if (!ReadFile(inputFileName, inputFile)) return;
  if (!IsInputFileSupported(inputFile)) return;

  PEFile::PEFile peLoader;
  if (!LoadPELoaderFromResource(peLoader, inputFile.IsConsole())) return;
  if (!RebasePELoaderIfRequired(inputFile, peLoader)) return;
  if (!SavePELoader(peLoader, outputFileName)) return;

  if (!AppendResourcesToPELoader(inputFileName, outputFileName)) return;
  if (!CompressAndAppendToPELoaderAsResource(inputFile, outputFileName)) return;

  Console::Console console;
  console.WriteLine(L"Packing finished.");
}

// Experimental approach: Requirement: .reloc section is last section of loader stub
void PackIntoLastSection(LPWSTR inputFileName, LPWSTR outputFileName)
{
  PEFile::PEFile inputFile;
  if (!ReadFile(inputFileName, inputFile)) return;
  if (!IsInputFileSupported(inputFile)) return;

  PEFile::PEFile peLoader;
  if (!LoadPELoaderFromResource(peLoader, inputFile.IsConsole())) return;
  if (!SavePELoader(peLoader, outputFileName)) return;
  if (!AppendResourcesToPELoader(inputFileName, outputFileName)) return;

  if (!ReadFile(outputFileName, peLoader)) return;
  if (!RebasePELoaderToLastSection(inputFile, peLoader)) return;
  if (!CompressAndAppendToPELoaderIntoLastSection(inputFile, peLoader)) return;
  if (!SavePELoader(peLoader, outputFileName)) return;

  Console::Console console;
  console.WriteLine(L"Packing finished.");
}

bool GetCommandLine(LPWSTR& inputFileName, LPWSTR& outputFileName, CommandLine::OPTION& option)
{
  Console::Console console;
  CommandLine::CommandLine commandLine;

  auto result = commandLine.Parse();
  if (!result.IsValid)
  {
    console.WriteError(result.ErrorMessage);
    console.ShowHelp();
    return false;
  }
  else
  {
    inputFileName = result.InputFileName;
    outputFileName = result.OutputFileName;
    option = result.Option;
    return true;
  }  
}

bool ReadFile(LPWSTR fileName, PEFile::PEFile& peFile)
{
  BinaryFileReader::BinaryFileReader binaryFileReader(fileName);
  if (binaryFileReader.GetBufferSize() == 0 || binaryFileReader.GetBuffer() == nullptr)
  {
    Console::Console console;
    console.WriteError(L"Reading file failed.");
    return false;
  }

  return peFile.LoadFromBuffer(binaryFileReader.GetBuffer(), binaryFileReader.GetBufferSize());
}

bool IsInputFileSupported(PEFile::PEFile& inputFile)
{
  Console::Console console;
  if (!inputFile.IsPEFile())
  {
    console.WriteError(L"inputfile is not a valid pe file.");
    return false;
  }
  if (!inputFile.Isx64())
  {
    console.WriteError(L"inputfile is not a x64 pe file.");
    return false;
  }
  if (!inputFile.IsNative())
  {
    console.WriteError(L"inputfile is not a native pe file.");
    return false;
  }
  return true;
}

bool LoadPELoaderFromResource(PEFile::PEFile& peLoader, bool useConsoleStub)
{
  Console::Console console;

  DWORD peLoaderSize = 0;
  ResourceLoader::ResourceLoader resourceLoader;
  BYTE* peLoaderBuffer = nullptr;
  if (useConsoleStub)
  {
    console.WriteLine(L"Using console loader stub.");
    peLoaderBuffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1000), RT_RCDATA, peLoaderSize); // 1000 : Loader_Console
  }
  else
  {
    console.WriteLine(L"Using windows loader stub.");
    peLoaderBuffer = resourceLoader.LoadResource(MAKEINTRESOURCE(1001), RT_RCDATA, peLoaderSize); // 1001 : Loader_Windows
  }

  if (peLoaderBuffer == nullptr || peLoaderSize == 0)
  {
    console.WriteError(L"Loading loader stub failed.");
    return false;
  }

  if (!peLoader.LoadFromBuffer(peLoaderBuffer, peLoaderSize)) return false;
  resourceLoader.Free(peLoaderBuffer);

  return true;
}

// TO DO
// Rebasing the pe loader to the next possible image base 'behind' the target is not the way to go.
// This calculated image base can still be occupied by stack, or by other loaded modules.
// Therefore i keep ASLR activated, for better results
bool RebasePELoaderIfRequired(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
{
  if (!inputFile.HasRelocationTable() && inputFile.IntersectsWith(peLoader))
  {
    Console::Console console;

    console.WriteLine(L"No relocation table found. Image base conflict detected. Rebasing...");
    ULONGLONG newImageBase = inputFile.GetNextImageBase();
    if (peLoader.Rebase(newImageBase)) // Rebase the pe loader to the next possible image base 'behind' the target
    {
      console.WriteLine(L"Rebasing finished.");
      return true;
    }
    else
    {
      console.WriteError(L"Rebasing failed.");
      return false;
    }
  }
  return true;
}

bool RebasePELoaderToLastSection(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
{
  Console::Console console;

  WORD lastIndex = peLoader.GetSectionCount() - 1;
  PIMAGE_SECTION_HEADER lastSectionHeader = peLoader.GetSectionHeader(lastIndex);
  uintptr_t newImageBase = inputFile.GetImageBase() - lastSectionHeader->VirtualAddress;
  uintptr_t alignedImageBase = PEFile::PEFile::AlignImageBase(newImageBase);
  if (!peLoader.Rebase(alignedImageBase))
  {
    console.WriteError(L"Rebasing failed.");
    return false;
  }

  PIMAGE_SECTION_HEADER previousSectionHeader = peLoader.GetSectionHeader(lastIndex - 1);
  uintptr_t gap = newImageBase - alignedImageBase;
  if (gap != 0)
  {
    previousSectionHeader->Misc.VirtualSize += gap;
  }

  lastSectionHeader->VirtualAddress = peLoader.AlignSection(previousSectionHeader->VirtualAddress + previousSectionHeader->Misc.VirtualSize);
  lastSectionHeader->Misc.VirtualSize = peLoader.AlignSection(inputFile.GetSizeOfImage());
  peLoader.NT_HEADERS()->OptionalHeader.SizeOfImage = peLoader.AlignSection(lastSectionHeader->VirtualAddress + lastSectionHeader->Misc.VirtualSize);

  console.WriteLine(L"Rebasing finished.");
  return true;
}

bool SavePELoader(PEFile::PEFile& peLoader, LPWSTR outputFileName)
{
  Console::Console console;

  // Write pe loader from resource to disk (Loader is added as resource in postbuild event)
  BinaryFileWriter::BinaryFileWriter binaryFileWriter;
  if (!binaryFileWriter.WriteFile(outputFileName, peLoader.GetBuffer(), peLoader.GetBufferSize()))
  {
    console.WriteError(L"Saving loader stub to disk failed.");
    return false;
  }
  return true;
}

bool AppendResourcesToPELoader(LPWSTR inputFileName, LPWSTR outputFileName)
{
  Console::Console console;

  // Extract icon from inputFile if available and add it to outputFile (pe loader)
  IconExtractor::IconExtractor iconExtractor;
  if (iconExtractor.ExtractAndSetIconWithCustomIds(inputFileName, outputFileName))
  {
    console.WriteLine(L"Adding icon finished.");
  }

  // Extract manifest from inputFile if available and add it to outputFile (pe loader)
  // This is required because the manifest can contain specific dll versions be loaded
  ManifestExtractor::ManifestExtractor manifestExtractor;
  if (manifestExtractor.ExtractManifestResources(inputFileName))
  {
    console.WriteLine(L"Manifest found. Adding manifest...");
    if (!manifestExtractor.AddManifestResourcesToTarget(outputFileName))
    {
      console.WriteError(L"Adding manifest failed.");
      return false;
    }
    else
    {
      console.WriteLine(L"Adding manifest finished.");
    }
  }
  return true;
}

bool CompressAndAppendToPELoaderAsResource(PEFile::PEFile& inputFile, LPWSTR outputFileName)
{
  Console::Console console;

  // Compress inputFile
  BYTE* compressed = nullptr;
  size_t compressedSize = 0;
  Compressor::Compressor compressor;
  if (!compressor.Compress(inputFile.GetBuffer(), inputFile.GetBufferSize(), &compressed, &compressedSize))
  {
    console.WriteError(L"Compressing failed.");
    return false;
  }

  // Append compressed data as resource to loader (outputFile)
  HANDLE updateHandle = BeginUpdateResourceW(outputFileName, FALSE);
  if (updateHandle == nullptr)
  {
    console.WriteError(L"Appending compressed data failed.");
    return false;
  }
  if (UpdateResource(updateHandle, RT_RCDATA, L"FPACK", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), compressed, (DWORD)compressedSize) == FALSE)
  {
    console.WriteError(L"Appending compressed data failed.");
    return false;
  }
  if (EndUpdateResource(updateHandle, FALSE) == FALSE)
  {
    console.WriteError(L"Appending compressed data failed.");
    return false;
  }

  compressor.Free(compressed);

  return true;
}

bool CompressAndAppendToPELoaderIntoLastSection(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
{
  Console::Console console;

  // Compress inputFile
  BYTE* compressed = nullptr;
  size_t compressedSize = 0;
  Compressor::Compressor compressor;
  if (!compressor.Compress(inputFile.GetBuffer(), inputFile.GetBufferSize(), &compressed, &compressedSize))
  {
    console.WriteError(L"Compressing failed.");
    return false;
  }

  WORD lastIndex = peLoader.GetSectionCount() - 1;
  PIMAGE_SECTION_HEADER lastSection = peLoader.GetSectionHeader(lastIndex);

  DWORD currentBufferSize = peLoader.GetBufferSize();
  DWORD currentSectionRawSize = lastSection->SizeOfRawData;
  DWORD newSectionRawSize = peLoader.AlignFile(compressedSize);
  DWORD deltaSize = newSectionRawSize - currentSectionRawSize;

  DWORD newBufferSize = currentBufferSize + deltaSize;
  lastSection->SizeOfRawData = newSectionRawSize;

  if (!peLoader.Resize(newBufferSize))
  {
    console.WriteError(L"Appending compressed data failed.");
    return false;
  }
  else
  {
    lastSection = peLoader.GetSectionHeader(lastIndex);
    memcpy(peLoader.GetBuffer() + lastSection->PointerToRawData, compressed, compressedSize);
    memcpy(lastSection->Name, ".fpack  ", 8);
  }
  
  compressor.Free(compressed);

  return true;
}
