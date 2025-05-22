#pragma once
#include <mini/common.h>

#include <cstdarg>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#endif

namespace mini::console {

void
write(
  const char* format,
  ...
  );

void
write_args(
  const char* format,
  va_list args
  );

#ifdef MINI_OS_WINDOWS
void
write_with_color(
  WORD color,
  const char* format,
  ...
  );

void
write_with_color_args(
  WORD color,
  const char* format,
  va_list args
  );

WORD
get_color(
  void
  );

void
set_color(
  WORD color
  );
#else
void
write_with_color(
  const char* color,
  const char* format,
  ...
  );

void
write_with_color_args(
  const char* color,
  const char* format,
  va_list args
  );

const char*
get_color(
  void
  );

void
set_color(
  const char* color
  );

// Définition des couleurs ANSI pour Linux
namespace colors {
  static constexpr const char* black         = "\033[30m";
  static constexpr const char* red           = "\033[31m";
  static constexpr const char* green         = "\033[32m";
  static constexpr const char* yellow        = "\033[33m";
  static constexpr const char* blue          = "\033[34m";
  static constexpr const char* magenta       = "\033[35m";
  static constexpr const char* cyan          = "\033[36m";
  static constexpr const char* white         = "\033[37m";
  static constexpr const char* bright_black  = "\033[90m";
  static constexpr const char* bright_red    = "\033[91m";
  static constexpr const char* bright_green  = "\033[92m";
  static constexpr const char* bright_yellow = "\033[93m";
  static constexpr const char* bright_blue   = "\033[94m";
  static constexpr const char* bright_magenta= "\033[95m";
  static constexpr const char* bright_cyan   = "\033[96m";
  static constexpr const char* bright_white  = "\033[97m";
  static constexpr const char* reset         = "\033[0m";
}
#endif

}