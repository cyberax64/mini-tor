#include "base16.h"
#include "detail/base_encode.h"

#ifdef MINI_OS_WINDOWS
#include <wincrypt.h>
#else
// Définitions des constantes équivalentes à celles de Windows
#define CRYPT_STRING_HEXRAW 0x00000004
#define CRYPT_STRING_NOCRLF 0x40000000
#define CRYPT_STRING_HEX_ANY 0x00000008
#endif

namespace mini::crypto::capi {

static void
encode_impl(
  const byte_type* input,
  size_type input_size,
  char* output,
  size_type& output_size
  )
{
  //
  // note that CRYPT_STRING_HEXRAW is not supported on Windows XP.
  //

  detail::base_encode_impl(
    CRYPT_STRING_HEXRAW | CRYPT_STRING_NOCRLF,
    input,
    input_size,
    output,
    output_size);
}

static void
decode_impl(
  const char* input,
  size_type input_size,
  byte_type* output,
  size_type& output_size
  )
{
  //
  // CRYPT_STRING_HEX_ANY:
  //
  // Tries the following, in order:
  //   CRYPT_STRING_HEXADDR
  //   CRYPT_STRING_HEXASCIIADDR
  //   CRYPT_STRING_HEX
  //   CRYPT_STRING_HEXRAW
  //   CRYPT_STRING_HEXASCII
  //

  detail::base_decode_impl(
    CRYPT_STRING_HEX_ANY,
    input,
    input_size,
    output,
    output_size);
}

string
base16::encode(
  const byte_buffer_ref input
  )
{
  string output;
  size_type output_size;
  encode_impl(input.get_buffer(), input.get_size(), nullptr, output_size);

  output.resize(output_size - 1);
  encode_impl(input.get_buffer(), input.get_size(), output.get_buffer(), output_size);

  return output;
}

byte_buffer
base16::decode(
  const string_ref input
  )
{
  byte_buffer output;
  size_type output_size;

  decode_impl(input.get_buffer(), input.get_size(), nullptr, output_size);

  output.resize(output_size);
  decode_impl(input.get_buffer(), input.get_size(), output.get_buffer(), output_size);

  return output;
}

}
