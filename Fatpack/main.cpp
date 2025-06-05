#include "Console\Console.h"
#include "CommandLine\CommandLine.h"
#include "PEFile\PEFile.h"
#include "..\Shared\BinaryFileReader\BinaryFileReader.h"
#include "BinaryFileWriter\BinaryFileWriter.h"
#include "IconExtractor\IconExtractor.h"
#include "ManifestExtractor\ManifestExtractor.h"
#include "..\Shared\ResourceLoader\ResourceLoader.h"
#include "Compessor\Compressor.h"

int main()
{
  Console::Console console;
  CommandLine::CommandLine commandLine;

  auto args = commandLine.GetCommandLine();

  int argc = 0;
  auto argv = commandLine.CommandLineToArgv(args, argc);
  if (argc != 3)
  {
    console.WriteLine(L"\n..::[Fatmike 2025]::..\n");
    console.WriteLine(L"Version: Fatpack v1.1.0");
    console.WriteLine(L"Usage:\t fatpack.exe inputfile.exe outputfile.exe");
    return 0;
  }

  LPWSTR inputFileName = argv[1];
  LPWSTR outputFileName = argv[2];

  if (wcscmp(inputFileName, outputFileName) == 0)
  {
    console.WriteLine(L"inputfile may not be the same as outputfile");
    return 0;
  }

  // Read input file
  BinaryFileReader::BinaryFileReader binaryFileReader(inputFileName);
  if (binaryFileReader.GetBufferSize() == 0 || binaryFileReader.GetBuffer() == nullptr)
  {
    console.WriteLine(L"Reading inputfile failed.");
    return 0;
  }

  PEFile::PEFile inputFile;
  inputFile.LoadFromBuffer(binaryFileReader.GetBuffer(), binaryFileReader.GetBufferSize());
  
  // Load PE Loader from resource (PE Loader is added as resource in postbuild event of ResourceAdder)
  DWORD peLoaderSize = 0;
  ResourceLoader::ResourceLoader resourceLoader;
  BYTE* peLoaderBuffer = nullptr;
  if (inputFile.IsConsole())
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
    return 0;
  }

  // Rebase pe loader if required
  PEFile::PEFile peLoader;
  peLoader.LoadFromBuffer(peLoaderBuffer, peLoaderSize);
  resourceLoader.Free(peLoaderBuffer);
  if (!inputFile.HasRelocationTable() && inputFile.GetImageBase() == peLoader.GetImageBase())
  {
    console.WriteLine(L"No relocation table found. Image base conflict detected. Rebasing...");
    ULONGLONG newImageBase = inputFile.GetNextImageBase();
    if (peLoader.Rebase(newImageBase)) // Rebase the pe loader to the next possible image base 'behind' the target
    {
      console.WriteLine(L"Rebasing finished.");
    }
    else
    {
      console.WriteLine(L"Rebasing failed.");
      return 0;
    }
  }
  
  // Write loader from resource to disk (Loader is added as resource in postbuild event of ResourceAdder)
  BinaryFileWriter::BinaryFileWriter binaryFileWriter;
  if (!binaryFileWriter.WriteFile(outputFileName, peLoader.GetBuffer(), peLoader.GetBufferSize()))
  {
    console.WriteLine(L"Saving loader stub to disk failed.");
    return 0;
  }

  // Extract icon from inputFile if available and add it to outputFile
  IconExtractor::IconExtractor iconExtractor;
  iconExtractor.ExtractAndSetIconWithCustomIds(inputFileName, outputFileName);

  // Extract manifest from inputFile if available and add it to outputFile
  // This is required cause the manifest can contain important data for loading the pe file.
  // For example a specific version of a dll to be loaded (Example: Version 6.0.0.0 of comctl32.dll since TaskDialogIndirect is only avaliable in this version)
  ManifestExtractor::ManifestExtractor manifestExtractor;
  if (manifestExtractor.ExtractManifestResources(inputFileName))
  {
    console.WriteLine(L"Manifest found. Adding manifest...");
    if (!manifestExtractor.AddManifestResourcesToTarget(outputFileName))
    {
      console.WriteLine(L"Adding manifest failed.");
    }
    else
    {
      console.WriteLine(L"Adding manifest finished.");
    }
  }
  
  // Compress input file
  BYTE* compressed = nullptr;
  size_t compressedSize = 0;
  Compressor::Compressor compressor;  
  compressor.Compress(inputFile.GetBuffer(), inputFile.GetBufferSize(), &compressed, &compressedSize);

  // Add packet target (inputFile) as resource to loader (ouputfile)
  HANDLE updateHandle = BeginUpdateResourceW(outputFileName, FALSE);
  UpdateResource(updateHandle, RT_RCDATA, L"PACKED", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), compressed, (DWORD)compressedSize);
  EndUpdateResource(updateHandle, FALSE);
  free(compressed);

  console.WriteLine(L"Packing finished.");

  return 0;
}
