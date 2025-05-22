#include "thread.h"

namespace mini::threading {

namespace detail {

void
thread_dispatcher(
  thread* thread_instance
  )
{
  thread_instance->thread_procedure();
}

#ifdef MINI_OS_WINDOWS
DWORD WINAPI
native_thread_dispatcher(
  LPVOID lpParam
  )
{
  thread_dispatcher(reinterpret_cast<thread*>(lpParam));
  return 0;
}
#else
void*
native_thread_dispatcher(
  void* lpParam
  )
{
  thread_dispatcher(reinterpret_cast<thread*>(lpParam));
  return nullptr;
}
#endif

}

//
// constructors.
//

thread::thread(
  thread&& other
  )
{
  swap(other);
}

thread::thread(
  current_thread_tag
  )
#ifdef MINI_OS_WINDOWS
  : _thread_handle(GetCurrentThread())
  , _thread_id(GetCurrentThreadId())
#else
  : _thread_handle(pthread_self())
  , _thread_id(static_cast<uint32_t>(pthread_self()))
#endif
{

}

//
// destructor
//

thread::~thread(
  void
  )
{
  stop();
}

//
// swap.
//

void
thread::swap(
  thread& other
  )
{
  mini::swap(_thread_handle, other._thread_handle);
  mini::swap(_thread_id, other._thread_id);
}

//
// operations.
//

void
thread::start(
  void
  )
{
  if (_thread_handle != 0)
  {
    //
    // thread is already running.
    //
    return;
  }

  if (_has_been_terminated)
  {
    //
    // do not start terminated thread again.
    //
    return;
  }

#ifdef MINI_OS_WINDOWS
  _thread_handle = CreateThread(
    NULL,                               // default security attributes
    0,                                  // use default stack size
    &detail::native_thread_dispatcher,  // thread function name
    (LPVOID)this,                       // argument to thread function
    0,                                  // use default creation flags
    &_thread_id);                       // returns the thread identifier
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  int result = pthread_create(
    &_thread_handle,                    // thread handle
    &attr,                              // thread attributes
    &detail::native_thread_dispatcher,  // thread function
    static_cast<void*>(this));          // argument to thread function
  
  if (result == 0) {
    _thread_id = static_cast<uint32_t>(_thread_handle);
  }
  
  pthread_attr_destroy(&attr);
#endif
}

void
thread::stop(
  void
  )
{
  if (_thread_handle == 0)
  {
    //
    // thread is not running or is already terminated.
    //
    return;
  }

#ifdef MINI_OS_WINDOWS
  TerminateThread(_thread_handle, 0);
  CloseHandle(_thread_handle);
#else
  // Sous Linux, on ne peut pas terminer un thread de manière aussi brutale
  // On envoie un signal pour demander la terminaison
  pthread_cancel(_thread_handle);
  pthread_join(_thread_handle, nullptr);
#endif

  _thread_handle = 0;
  _thread_id = 0;
  _has_been_terminated = true;
}

wait_result
thread::join(
  timeout_type timeout
  )
{
#ifdef MINI_OS_WINDOWS
  return static_cast<wait_result>(WaitForSingleObject(
    _thread_handle,
    (DWORD)timeout
    ));
#else
  if (timeout == wait_infinite) {
    // Attente infinie
    int result = pthread_join(_thread_handle, nullptr);
    return result == 0 ? wait_result::success : wait_result::failed;
  } else {
    // Attente avec timeout
    struct timespec ts;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    
    // Convertir le timeout en secondes et nanosecondes
    ts.tv_sec = tv.tv_sec + (timeout / 1000);
    ts.tv_nsec = (tv.tv_usec + (timeout % 1000) * 1000) * 1000;
    
    // Normaliser les nanosecondes
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }
    
    int result = pthread_timedjoin_np(_thread_handle, nullptr, &ts);
    
    if (result == 0) {
      return wait_result::success;
    } else if (result == ETIMEDOUT) {
      return wait_result::timeout;
    } else {
      return wait_result::failed;
    }
  }
#endif
}

//
// observers.
//

thread::id
thread::get_id(
  void
  ) const
{
  return _thread_id;
}

bool
thread::is_alive(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return
    _thread_handle != 0 &&
    WaitForSingleObject(_thread_handle, 0) == WAIT_TIMEOUT;
#else
  if (_thread_handle == 0) {
    return false;
  }
  
  // Vérifier si le thread existe encore
  int result = pthread_kill(_thread_handle, 0);
  return result == 0;
#endif
}

//
// static methods.
//

thread
thread::get_current_thread(
  void
  )
{
  return thread(current_thread_tag());
}

void
thread::sleep(
  timeout_type milliseconds
  )
{
#ifdef MINI_OS_WINDOWS
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000); // usleep prend des microsecondes
#endif
}

//
// virtual methods.
//

void
thread::thread_procedure(
  void
  )
{

}

void
swap(
  thread& lhs,
  thread& rhs
  )
{
  lhs.swap(rhs);
}

}
