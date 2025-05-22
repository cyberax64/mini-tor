#include "logger.h"
#include "console.h"

namespace mini {

logger log;

#ifdef MINI_OS_WINDOWS
static constexpr WORD level_colors[] = {
  FOREGROUND_INTENSITY | FOREGROUND_GREEN /* | FOREGROUND_BLUE */,  // debug
  FOREGROUND_INTENSITY | FOREGROUND_GREEN,                    // info
  FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,   // warning
  FOREGROUND_INTENSITY | FOREGROUND_RED,                      // error
};
#else
// ANSI color codes for Linux
static constexpr const char* level_colors[] = {
  "\033[1;32m",  // debug - bright green
  "\033[0;32m",  // info - green
  "\033[1;33m",  // warning - bright yellow
  "\033[1;31m",  // error - bright red
};
static constexpr const char* color_reset = "\033[0m";
#endif

void
logger::log(
  level l,
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  log_args(l, format, args);
  va_end(args);
}

void
logger::log_args(
  level l,
  const char* format,
  va_list args
  )
{
  if (l >= _level)
  {
#ifdef MINI_OS_WINDOWS
    SYSTEMTIME local_time;
    GetLocalTime(&local_time);

    console::write_with_color(
      level_colors[(int)l],
      "[%02u:%02u:%02u.%03u] ",
      local_time.wHour,
      local_time.wMinute,
      local_time.wSecond,
      local_time.wMilliseconds);

    console::write_with_color_args(level_colors[(int)l], format, args);
#else
    // Linux implementation
    struct timeval tv;
    struct tm* tm_info;
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    // Print timestamp with color
    fprintf(stdout, "%s[%02d:%02d:%02d.%03d] ", 
            level_colors[(int)l],
            tm_info->tm_hour,
            tm_info->tm_min, 
            tm_info->tm_sec,
            (int)(tv.tv_usec / 1000));
    
    // Print message with color and reset
    vfprintf(stdout, format, args);
    fprintf(stdout, "%s", color_reset);
    fflush(stdout);
#endif
  }
}

void
logger::debug(
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  log(level::debug, format, args);
  va_end(args);
}

void
logger::info(
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  log(level::info, format, args);
  va_end(args);
}

void
logger::warning(
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  log(level::warning, format, args);
  va_end(args);
}

void
logger::error(
  const char* format,
  ...
  )
{
  va_list args;
  va_start(args, format);
  log(level::error, format, args);
  va_end(args);
}

logger::level
logger::get_level(
  void
  ) const
{
  return _level;
}

void
logger::set_level(
  level new_level
  )
{
  _level = new_level;
}

}
