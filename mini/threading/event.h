#pragma once
#include "common.h"

#include <mini/common.h>
#include <mini/time.h>
#include <mini/buffer_ref.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <sys/time.h>
#endif

namespace mini::threading {

enum class reset_type : uint8_t
{
  auto_reset,
  manual_reset,
};

class event
{
  MINI_MAKE_NONCOPYABLE(event);

  public:
    event(
      event&& other
      ) = default;

    event(
      reset_type type = reset_type::manual_reset,
      bool initial_state = false
      );

    ~event(
      void
      );

    void
    set(
      void
      );

    void
    reset(
      void
      );

    wait_result
    wait(
      timeout_type timeout = wait_infinite
      );

    static wait_result
    wait_for_all(
      buffer_ref<const event*> events,
      timeout_type timeout = wait_infinite
      );

    static wait_result
    wait_for_any(
      buffer_ref<const event*> events,
      timeout_type timeout = wait_infinite
      );

    static int
    index_from_wait_result(
      wait_result result
      );

    bool
    is_signaled(
      void
      );

  private:
#ifdef MINI_OS_WINDOWS
    HANDLE _event;
#else
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
    bool _signaled;
    bool _is_auto_reset;
#endif
};

}
