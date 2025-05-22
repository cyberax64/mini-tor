#include "base_encode.h"

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#include <wincrypt.h>
#else
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// Définitions des constantes équivalentes à celles de Windows
#define CRYPT_STRING_BASE64 0x00000001
#define CRYPT_STRING_HEXRAW 0x00000004
#define CRYPT_STRING_HEXASCIIADDR 0x00000008
#endif

namespace mini::crypto::capi::detail {

void
base_encode_impl(
  DWORD flags,
  const byte_type* input,
  size_type input_size,
  char* output,
  size_type& output_size
  )
{
#ifdef MINI_OS_WINDOWS
  if (!output)
  {
    output_size = 0;
  }

  CryptBinaryToString(
    input,
    static_cast<DWORD>(input_size),
    flags,
    output,
    reinterpret_cast<DWORD*>(&output_size));
#else
  if (!output)
  {
    // Calculer la taille nécessaire
    if (flags == CRYPT_STRING_BASE64)
    {
      // Base64: 4 caractères pour 3 octets, plus padding
      output_size = ((input_size + 2) / 3) * 4 + 1; // +1 pour le caractère nul
    }
    else if (flags == CRYPT_STRING_HEXRAW || flags == CRYPT_STRING_HEXASCIIADDR)
    {
      // Hex: 2 caractères par octet
      output_size = input_size * 2 + 1; // +1 pour le caractère nul
    }
    else
    {
      output_size = 0;
    }
    return;
  }

  if (flags == CRYPT_STRING_BASE64)
  {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, input_size);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    memcpy(output, bufferPtr->data, bufferPtr->length);
    output[bufferPtr->length] = '\0';
    output_size = bufferPtr->length + 1;

    BIO_free_all(bio);
  }
  else if (flags == CRYPT_STRING_HEXRAW || flags == CRYPT_STRING_HEXASCIIADDR)
  {
    static const char hex_chars[] = "0123456789ABCDEF";
    for (size_type i = 0; i < input_size; i++)
    {
      output[i * 2] = hex_chars[(input[i] >> 4) & 0xF];
      output[i * 2 + 1] = hex_chars[input[i] & 0xF];
    }
    output[input_size * 2] = '\0';
    output_size = input_size * 2 + 1;
  }
  else
  {
    output_size = 0;
  }
#endif
}

void
base_decode_impl(
  DWORD flags,
  const char* input,
  size_type input_size,
  byte_type* output,
  size_type& output_size
  )
{
#ifdef MINI_OS_WINDOWS
  if (!output)
  {
    output_size = 0;
  }

  CryptStringToBinary(
    input,
    static_cast<DWORD>(input_size),
    flags,
    output,
    reinterpret_cast<DWORD*>(&output_size),
    NULL,
    NULL);
#else
  if (!output)
  {
    // Calculer la taille nécessaire
    if (flags == CRYPT_STRING_BASE64)
    {
      // Base64: 3 octets pour 4 caractères
      output_size = (input_size / 4) * 3;
    }
    else if (flags == CRYPT_STRING_HEXRAW || flags == CRYPT_STRING_HEXASCIIADDR)
    {
      // Hex: 1 octet pour 2 caractères
      output_size = input_size / 2;
    }
    else
    {
      output_size = 0;
    }
    return;
  }

  if (flags == CRYPT_STRING_BASE64)
  {
    BIO *bio, *b64;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, input_size);
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    output_size = BIO_read(bio, output, input_size);
    
    BIO_free_all(bio);
  }
  else if (flags == CRYPT_STRING_HEXRAW || flags == CRYPT_STRING_HEXASCIIADDR)
  {
    size_type j = 0;
    for (size_type i = 0; i < input_size; i += 2)
    {
      char high = input[i];
      char low = input[i + 1];
      
      // Convertir les caractères hexadécimaux en valeurs
      high = (high >= '0' && high <= '9') ? high - '0' : 
             (high >= 'A' && high <= 'F') ? high - 'A' + 10 : 
             (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;
      
      low = (low >= '0' && low <= '9') ? low - '0' : 
            (low >= 'A' && low <= 'F') ? low - 'A' + 10 : 
            (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;
      
      output[j++] = (high << 4) | low;
    }
    output_size = j;
  }
  else
  {
    output_size = 0;
  }
#endif
}

}
