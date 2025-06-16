#pragma once

namespace Packer
{
  enum class Error
  {
    Ok,
    Error_Read_File,                // L"Failed to read file."
    Error_Load_Pe_From_Buffer,      // L"Failed to load PE file from buffer."
    Error_Pe_Not_Valid,             // L"Input file is not a valid PE file."
    Error_Pe_Not_x64,               // L"Input file is not a x64 PE file."
    Error_Pe_Not_Native,            // L"Input file is not a native PE file."
    Error_Loading_Loader_Stub,      // L"Loading loader stub failed."
    Error_Saving_Pe,                // L"Failed to save PE file."
    Error_Adding_Icon,              // L"Adding icon failed."
    Error_Adding_Manifest,          // L"Adding manifest failed."
    Error_Rebasing,                 // L"Rebasing failed."
    Error_Compressing,              // L"Compressing failed."
    Error_Appending_Compressed_Data // L"Appending compressed data failed."
  };
}