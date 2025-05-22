#pragma once

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#endif

namespace mini::threading {

#ifdef MINI_OS_WINDOWS
enum class wait_result : DWORD
{
  success   = WAIT_OBJECT_0,
  abandoned = WAIT_ABANDONED_0,
  timeout   = WAIT_TIMEOUT,
  failed    = WAIT_FAILED,
};
#else
enum class wait_result : int
{
  success   = 0,
  abandoned = 1,
  timeout   = ETIMEDOUT,
  failed    = -1,
};

#endif

#define mini_wait_success(result) ((result) == ::mini::threading::wait_result::success)

}
