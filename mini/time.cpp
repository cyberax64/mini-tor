#include "time.h"

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#include <winternl.h>
#else
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

// struct tm
// {
//   int tm_sec;   // seconds after the minute - [0, 60] including leap second
//   int tm_min;   // minutes after the hour - [0, 59]
//   int tm_hour;  // hours since midnight - [0, 23]
//   int tm_mday;  // day of the month - [1, 31]
//   int tm_mon;   // months since January - [0, 11]
//   int tm_year;  // years since 1900
//   int tm_wday;  // days since Sunday - [0, 6]
//   int tm_yday;  // days since January 1 - [0, 365]
//   int tm_isdst; // daylight savings time flag
// };

#ifdef MINI_OS_WINDOWS
extern "C"
uint32_t __cdecl
time(
  uint32_t* _Time
  );

extern "C"
uint32_t __cdecl
_mkgmtime(
  struct tm* timeptr
  );
#endif

namespace mini {

//
// constructors.
//

time::time(
  void
  )
  : time(0)
{

}

time::time(
  uint32_t value
  )
  : _timestamp(value)
{

}

//
// conversion.
//

void
time::parse(
  const string_ref value
  )
{
  //
  // must be in format "2016-06-14 01:00:00"
  //

  tm system_time = { 0 };
  sscanf(
    value.get_buffer(),
    "%04d-%02d-%02d %02d:%02d:%02d",
    &system_time.tm_year,
    &system_time.tm_mon,
    &system_time.tm_mday,
    &system_time.tm_hour,
    &system_time.tm_min,
    &system_time.tm_sec);

  system_time.tm_year -=  1900;
  system_time.tm_mon  -=     1;
  system_time.tm_isdst =    -1;

#ifdef MINI_OS_WINDOWS
  _timestamp = _mkgmtime(&system_time);
#else
  _timestamp = timegm(&system_time);
#endif
}

uint32_t
time::to_timestamp(
  void
  ) const
{
  return _timestamp;
}

//
// static methods.
//

time
time::now(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  return ::time(nullptr);
#else
  return ::time(nullptr);
#endif
}

timestamp_type
time::timestamp(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  return GetTickCount();
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
}

//
// operators.
//

bool
operator==(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp == rhs._timestamp;
}

bool
operator!=(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp != rhs._timestamp;
}

bool
operator<(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp < rhs._timestamp;
}

bool
operator>(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp > rhs._timestamp;
}

bool
operator<=(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp <= rhs._timestamp;
}

bool
operator>=(
  const time& lhs,
  const time& rhs
  )
{
  return lhs._timestamp >= rhs._timestamp;
}

};

