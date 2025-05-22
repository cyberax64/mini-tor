#pragma once
#include <mini/common.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

#define mini_lock(mutex_instance)                             \
  if (::mini::threading::mutex_holder __h = mutex_instance)   \
  {                                                           \
    MINI_UNREACHABLE;                                         \
  }                                                           \
  else

#define mini_unlock(mutex_instance)                           \
  if (::mini::threading::mutex_unholder __h = mutex_instance) \
  {                                                           \
    MINI_UNREACHABLE;                                         \
  }                                                           \
  else

namespace mini::threading {

class mutex
{
  MINI_MAKE_NONCOPYABLE(mutex);

  public:
    mutex(
      void
      );

    mutex(
      mutex&& other
      ) = default;

    ~mutex(
      void
      );

    void
    acquire(
      void
      );

    void
    release(
      void
      );

  private:
#ifdef MINI_OS_WINDOWS
    CRITICAL_SECTION _critical_section;
#else
    pthread_mutex_t _mutex;
#endif
};

class mutex_holder
{
  MINI_MAKE_NONCOPYABLE(mutex_holder);

  public:
    mutex_holder(
      mutex_holder&& other
      ) = default;

    mutex_holder(
      mutex& mutex
      );

    ~mutex_holder(
      void
      );

    operator bool(
      void
      ) const;

  private:
    mutex& _mutex;
};

class mutex_unholder
{
  MINI_MAKE_NONCOPYABLE(mutex_unholder);

  public:
    mutex_unholder(
      mutex_unholder&& other
      ) = default;

    mutex_unholder(
      mutex& mutex
      );

    ~mutex_unholder(
      void
      );

    operator bool(
      void
      ) const;

  private:
    mutex& _mutex;
};

}