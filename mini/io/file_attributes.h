#pragma once
#include <mini/common.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
#include <sys/stat.h>
// Définitions des constantes Windows pour Linux
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_DEVICE 0x00000040
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#define FILE_ATTRIBUTE_SPARSE_FILE 0x00000200
#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400
#define FILE_ATTRIBUTE_COMPRESSED 0x00000800
#define FILE_ATTRIBUTE_OFFLINE 0x00001000
#endif

namespace mini::io {

class file_attributes
{
  public:
    static const file_attributes invalid;

    file_attributes(
      void
      );

    file_attributes(
      uint32_t attributes
      );

    file_attributes(
      const file_attributes& other
      ) = default;

    file_attributes(
      file_attributes&& other
      ) = default;

    ~file_attributes(
      void
      ) = default;

    file_attributes&
    operator=(
      const file_attributes& other
      ) = default;

    file_attributes&
    operator=(
      file_attributes&& other
      ) = default;

    bool
    operator==(
      const file_attributes& other
      ) const;

    bool
    operator!=(
      const file_attributes& other
      ) const;

    bool
    is_directory(
      void
      ) const;

    bool
    is_file(
      void
      ) const;

    bool
    is_system(
      void
      ) const;

    bool
    is_hidden(
      void
      ) const;

  private:
    uint32_t _attributes;
};

}