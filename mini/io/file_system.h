#pragma once
#include <mini/common.h>
#include <mini/string.h>
#include "file_attributes.h"

namespace mini::io {

class file_system
{
public:
  static file_attributes
  get_attributes(
    const string_ref path
    );

  static bool
  exists(
    const string_ref path
    );

  static bool
  create_directory(
    const string_ref path
    );

  static bool
  remove_directory(
    const string_ref path
    );

  static bool
  remove_file(
    const string_ref path
    );

  static bool
  rename(
    const string_ref old_path,
    const string_ref new_path
    );
};

}