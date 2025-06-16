#pragma once

namespace Error
{
  enum class ErrorCode
  {
    Ok = 0,
    Error_Argument_Count,
    Error_Equal_Filenames,
    Error_Unknown_Option,
    Error_Read_File,
    Error_Load_Pe_From_Buffer,
    Error_Pe_Not_Valid,
    Error_Pe_Not_x64,
    Error_Pe_Not_Native,
    Error_Loading_Loader_Stub,
    Error_Saving_Pe,
    Error_Adding_Icon,
    Error_Adding_Manifest,
    Error_Rebasing,
    Error_Compressing,
    Error_Appending_Compressed_Data
  };
}