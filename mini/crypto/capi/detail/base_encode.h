#pragma once
#include <mini/string.h>
#include <mini/byte_buffer.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
// Pour Linux, définir DWORD comme un type équivalent
typedef unsigned long DWORD;
#endif

namespace mini::crypto::capi::detail {

void
base_encode_impl(
  DWORD flags,
  const byte_type* input,
  size_type input_size,
  char* output,
  size_type& output_size
  );

void
base_decode_impl(
  DWORD flags,
  const char* input,
  size_type input_size,
  byte_type* output,
  size_type& output_size
  );

}
