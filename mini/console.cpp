#include "console.h"

#include <mini/threading/mutex.h>

//
// functions
//

#ifndef MINI_MSVCRT_LIB
# include <cstdio>
# include <cstdlib>
#else

extern "C" {

int sscanf(
  const char* buffer,
  const char* format,
  ...
  );

int printf(
  const char* format,
  ...
  );

}

#endif

namespace mini::console {

threading::mutex g_console_output_mutex;

void
write(
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  write_args(format, args);
  va_end(args);
}

#ifdef MINI_OS_WINDOWS
void
write_with_color(
  WORD color,
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  write_with_color_args(color, format, args);
  va_end(args);
}
#else
void
write_with_color(
  const char* color,
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  write_with_color_args(color, format, args);
  va_end(args);
}
#endif

void
write_args(
  const char* format,
  va_list args
  )
{
  mini_lock(g_console_output_mutex)
  {
    vprintf(format, args);
  }
}

#ifdef MINI_OS_WINDOWS
void
write_with_color_args(
  WORD color,
  const char* format,
  va_list args
  )
{
  mini_lock(g_console_output_mutex)
  {
    WORD previous_color = get_color();
    set_color(color);
    vprintf(format, args);
    set_color(previous_color);
  }
}

WORD
get_color(
  void
  )
{
  CONSOLE_SCREEN_BUFFER_INFO console_info;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);

  return console_info.wAttributes;
}

void
set_color(
  WORD color
  )
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
#else
// Linux implementation
static const char* current_color = "\033[0m";

void
write_with_color_args(
  const char* color,
  const char* format,
  va_list args
  )
{
  mini_lock(g_console_output_mutex)
  {
    const char* previous_color = get_color();
    set_color(color);
    vprintf(format, args);
    set_color(previous_color);
  }
}

const char*
get_color(
  void
  )
{
  return current_color;
}

void
set_color(
  const char* color
  )
{
  current_color = color;
  printf("%s", color);
}
#endif

}