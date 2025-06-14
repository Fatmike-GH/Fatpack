#include "Console\Console.h"
#include "CommandLine\CommandLine.h"
#include "PEFile\PEFile.h"
#include "..\Shared\BinaryFileReader\BinaryFileReader.h"
#include "BinaryFileWriter\BinaryFileWriter.h"
#include "IconExtractor\IconExtractor.h"
#include "ManifestExtractor\ManifestExtractor.h"
#include "..\Shared\ResourceLoader\ResourceLoader.h"
#include "Compessor\Compressor.h"

bool GetCommandLine(LPWSTR& inputFileName, LPWSTR& outputFileName);
bool ReadInputFile(LPWSTR inputFileName, PEFile::PEFile& inputFile);
bool IsInputFileSupported(PEFile::PEFile& inputFile);
bool LoadPELoaderFromResource(PEFile::PEFile& peLoader, bool useConsoleStub);
bool RebasePELoader(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader);
bool SavePELoader(PEFile::PEFile& peLoader, LPWSTR outputFileName);
bool AppendResourcesToPELoader(LPWSTR inputFileName, LPWSTR outputFileName);
bool CompressAndAppendToPELoader(PEFile::PEFile& inputFile, LPWSTR outputFileName);

int main()
{
  LPWSTR inputFileName = nullptr;
  LPWSTR outputFileName = nullptr;
  if (!GetCommandLine(inputFileName, outputFileName)) return 0;

  PEFile::PEFile inputFile;
  if (!ReadInputFile(inputFileName, inputFile)) return 0;
  if (!IsInputFileSupported(inputFile)) return 0;

  PEFile::PEFile peLoader;
  if (!LoadPELoaderFromResource(peLoader, inputFile.IsConsole())) return 0;
  if (!RebasePELoader(inputFile, peLoader)) return 0;
  if (!SavePELoader(peLoader, outputFileName)) return 0;

  if (!AppendResourcesToPELoader(inputFileName, outputFileName)) return 0;
  if (!CompressAndAppendToPELoader(inputFile, outputFileName)) return 0;
  
  Console::Console console;
  console.WriteLine(L"Packing finished.");

  return 0;
}

bool GetCommandLine(LPWSTR& inputFileName, LPWSTR& outputFileName)
{
  Console::Console console;
  CommandLine::CommandLine commandLine;

  auto args = commandLine.GetCommandLine();

  int argc = 0;
  auto argv = commandLine.CommandLineToArgv(args, argc);
  if (argc != 3)
  {
    console.WriteLine(L"\n..::[Fatmike 2025]::..\n");
    console.WriteLine(L"Version: Fatpack v1.4.1");
    console.WriteLine(L"Usage:\t fatpack.exe inputfile.exe outputfile.exe");
    return false;
  }

  inputFileName = argv[1];
  outputFileName = argv[2];

  if (wcscmp(inputFileName, outputFileName) == 0)
  {
    console.WriteLine(L"inputfile may not be the same as outputfile");
    return false;
  }

  return true;
}

bool ReadInputFile(LPWSTR inputFileName, PEFile::PEFile& inputFile)
{
  BinaryFileReader::BinaryFileReader binaryFileReader(inputFileName);
  if (binaryFileReader.GetBufferSize() == 0 || binaryFileReader.GetBuffer() == nullptr)
  {
    Console::Console console;
    console.WriteLine(L"Reading inputfile failed.");
    return false;
  }

  return inputFile.LoadFromBuffer(binaryFileReader.GetBuffer(), binaryFileReader.GetBufferSize());
}

bool IsInputFileSupported(PEFile::PEFile& inputFile)
{
  Console::Console console;
  if (!inputFile.IsPEFile())
  {
    console.WriteLine(L"inputfile is not a valid pe file.");
    return false;
  }
  if (!inputFile.Isx64())
  {
    console.WriteLine(L"inputfile is not a x64 pe file.");
    return false;
  }
  if (!inputFile.IsNative())
  {
    console.WriteLine(L"inputfile is not a native pe file.");
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
    console.WriteLine(L"Loading loader stub failed.");
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
bool RebasePELoader(PEFile::PEFile& inputFile, PEFile::PEFile& peLoader)
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
      console.WriteLine(L"Rebasing failed.");
      return false;
    }
  }
  return true;
}

bool SavePELoader(PEFile::PEFile& peLoader, LPWSTR outputFileName)
{
  Console::Console console;

  // Write pe loader from resource to disk (Loader is added as resource in postbuild event)
  BinaryFileWriter::BinaryFileWriter binaryFileWriter;
  if (!binaryFileWriter.WriteFile(outputFileName, peLoader.GetBuffer(), peLoader.GetBufferSize()))
  {
    console.WriteLine(L"Saving loader stub to disk failed.");
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
      console.WriteLine(L"Adding manifest failed.");
      return false;
    }
    else
    {
      console.WriteLine(L"Adding manifest finished.");
    }
  }
  return true;
}

bool CompressAndAppendToPELoader(PEFile::PEFile& inputFile, LPWSTR outputFileName)
{
  Console::Console console;

  // Compress inputFile
  BYTE* compressed = nullptr;
  size_t compressedSize = 0;
  Compressor::Compressor compressor;
  if (!compressor.Compress(inputFile.GetBuffer(), inputFile.GetBufferSize(), &compressed, &compressedSize))
  {
    console.WriteLine(L"Compressing failed.");
    return false;
  }

  // Append compressed data as resource to loader (outputFile)
  HANDLE updateHandle = BeginUpdateResourceW(outputFileName, FALSE);
  if (updateHandle == nullptr)
  {
    console.WriteLine(L"Appending compressed data failed.");
    return false;
  }
  if (UpdateResource(updateHandle, RT_RCDATA, L"PACKED", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), compressed, (DWORD)compressedSize) == FALSE)
  {
    console.WriteLine(L"Appending compressed data failed.");
    return false;
  }
  if (EndUpdateResource(updateHandle, FALSE) == FALSE)
  {
    console.WriteLine(L"Appending compressed data failed.");
    return false;
  }

  compressor.Free(compressed);

  return true;
}
