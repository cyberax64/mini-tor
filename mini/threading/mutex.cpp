#include "mutex.h"

namespace mini::threading {

//
// mutex implementation.
//

mutex::mutex(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  InitializeCriticalSection(&_critical_section);
#else
  pthread_mutex_init(&_mutex, nullptr);
#endif
}

mutex::~mutex(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  DeleteCriticalSection(&_critical_section);
#else
  pthread_mutex_destroy(&_mutex);
#endif
}

void
mutex::acquire(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  EnterCriticalSection(&_critical_section);
#else
  pthread_mutex_lock(&_mutex);
#endif
}

void
mutex::release(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  LeaveCriticalSection(&_critical_section);
#else
  pthread_mutex_unlock(&_mutex);
#endif
}

//
// mutex holder implementation.
//

mutex_holder::mutex_holder(
  mutex& mutex
  )
  : _mutex(mutex)
{
  _mutex.acquire();
}

mutex_holder::~mutex_holder(
  void
  )
{
  _mutex.release();
}

mutex_holder::operator bool(
  void
  ) const
{
  return false;
}

//
// mutex holder implementation.
//

mutex_unholder::mutex_unholder(
  mutex& mutex
  )
  : _mutex(mutex)
{
  _mutex.release();
}

mutex_unholder::~mutex_unholder(
  void
  )
{
  _mutex.acquire();
}

mutex_unholder::operator bool(
  void
  ) const
{
  return false;
}

}