#pragma once
#include "file_attributes.h"

#include <mini/string.h>
#include <mini/common.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Définitions des types Windows pour Linux
typedef void* HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
typedef struct _WIN32_FIND_DATAA {
    uint32_t dwFileAttributes;
    char cFileName[260];
    char cAlternateFileName[14];
    uint32_t nFileSizeLow;
    uint32_t nFileSizeHigh;
} WIN32_FIND_DATAA;
#endif

namespace mini::io {

//
// file_info.
//

struct file_info
{
  string full_name;
  file_size_type size;
  file_attributes attributes;
};

//
// file_iterator.
//

class file_iterator
{
  public:
    static const file_iterator invalid;

    file_iterator(
      void
      );

    file_iterator(
      const string_ref path
      );

    file_iterator&
    operator++(
      void
      );

    file_iterator
    operator++(
      int
      );

    bool
    operator==(
      const file_iterator& other
      );

    bool
    operator!=(
      const file_iterator& other
      );

    file_info&
    operator*(
      void
      );

    file_info*
    operator->(
      void
      );

  private:
    file_info _cached_file_info;
    WIN32_FIND_DATAA _find_data;
    HANDLE _find_handle;
    bool _is_invalid;
};

//
// file_enumerator.
//

class file_enumerator
{
  public:
    file_enumerator(
      const string_ref path
      );

    file_iterator
    begin(
      void
      ) const;

    file_iterator
    end(
      void
      ) const;

  private:
    string _path;
};

}
