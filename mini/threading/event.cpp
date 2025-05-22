#include "event.h"
#include "thread.h"
#include <mini/collections/list.h>

#include <type_traits>

namespace mini::threading {

event::event(
  reset_type type,
  bool initial_state
  )
{
#ifdef MINI_OS_WINDOWS
  _event = CreateEvent(NULL, (BOOL)type, initial_state, NULL);
#else
  pthread_mutex_init(&_mutex, nullptr);
  pthread_cond_init(&_cond, nullptr);
  _signaled = initial_state;
  _is_auto_reset = (type == reset_type::auto_reset);
#endif
}

event::~event(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  CloseHandle(_event);
#else
  pthread_mutex_destroy(&_mutex);
  pthread_cond_destroy(&_cond);
#endif
}

void
event::set(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  SetEvent(_event);
#else
  pthread_mutex_lock(&_mutex);
  _signaled = true;
  pthread_cond_broadcast(&_cond);
  pthread_mutex_unlock(&_mutex);
#endif
}

void
event::reset(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  ResetEvent(_event);
#else
  pthread_mutex_lock(&_mutex);
  _signaled = false;
  pthread_mutex_unlock(&_mutex);
#endif
}

wait_result
event::wait(
  timeout_type timeout
  )
{
#ifdef MINI_OS_WINDOWS
  return static_cast<wait_result>(WaitForSingleObject(_event, timeout));
#else
  pthread_mutex_lock(&_mutex);
  
  if (_signaled) {
    if (_is_auto_reset) {
      _signaled = false;
    }
    pthread_mutex_unlock(&_mutex);
    return wait_result::success;
  }
  
  if (timeout == 0) {
    pthread_mutex_unlock(&_mutex);
    return wait_result::timeout;
  }
  
  int result = 0;
  
  if (timeout == wait_infinite) {
    // Attente infinie
    while (!_signaled && result == 0) {
      result = pthread_cond_wait(&_cond, &_mutex);
    }
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
    
    while (!_signaled && result == 0) {
      result = pthread_cond_timedwait(&_cond, &_mutex, &ts);
    }
  }
  
  wait_result wait_result_value = wait_result::failed;
  
  if (_signaled) {
    if (_is_auto_reset) {
      _signaled = false;
    }
    wait_result_value = wait_result::success;
  } else if (result == ETIMEDOUT) {
    wait_result_value = wait_result::timeout;
  }
  
  pthread_mutex_unlock(&_mutex);
  return wait_result_value;
#endif
}

wait_result
event::wait_for_all(
  buffer_ref<const event*> events,
  timeout_type timeout
  )
{
#ifdef MINI_OS_WINDOWS
  collections::list<HANDLE> handles;

  for (auto&& event : events)
  {
    handles.add(event->_event);
  }

  return static_cast<wait_result>(WaitForMultipleObjects(
    static_cast<DWORD>(handles.get_size()),
    //
    // dirty hack - as long as 'event' class
    // consists only from one HANDLE member,
    // this should work just fine.
    //
    &handles[0],
    TRUE,
    timeout));
#else
  // Sous Linux, nous devons attendre chaque événement individuellement
  for (auto&& event_ptr : events)
  {
    wait_result result = event_ptr->wait(timeout);
    if (result != wait_result::success)
    {
      return result;
    }
  }
  
  return wait_result::success;
#endif
}

wait_result
event::wait_for_any(
  buffer_ref<const event*> events,
  timeout_type timeout
  )
{
#ifdef MINI_OS_WINDOWS
  collections::list<HANDLE> handles;

  for (auto&& event : events)
  {
    handles.add(event->_event);
  }

  return static_cast<wait_result>(WaitForMultipleObjects(
    static_cast<DWORD>(handles.get_size()),
    //
    // dirty hack - as long as 'event' class
    // consists only from one HANDLE member,
    // this should work just fine.
    //
    &handles[0],
    FALSE,
    timeout));
#else
  // Implémentation simplifiée pour Linux
  // Cette implémentation n'est pas efficace car elle vérifie séquentiellement
  // chaque événement, mais elle devrait fonctionner pour les besoins de base
  
  // Si timeout est 0, on vérifie juste une fois
  if (timeout == 0)
  {
    for (size_t i = 0; i < events.get_size(); i++)
    {
      if (events[i]->is_signaled())
      {
        // Retourne wait_result::success + index
        return static_cast<wait_result>(static_cast<int>(wait_result::success) + i);
      }
    }
    return wait_result::timeout;
  }
  
  // Sinon, on vérifie périodiquement
  const timeout_type check_interval = 10; // ms
  timeout_type elapsed = 0;
  
  while (timeout == wait_infinite || elapsed < timeout)
  {
    for (size_t i = 0; i < events.get_size(); i++)
    {
      if (events[i]->is_signaled())
      {
        // Retourne wait_result::success + index
        return static_cast<wait_result>(static_cast<int>(wait_result::success) + i);
      }
    }
    
    // Attendre un peu avant de vérifier à nouveau
    thread::sleep(check_interval);
    
    if (timeout != wait_infinite)
    {
      elapsed += check_interval;
    }
  }
  
  return wait_result::timeout;
#endif
}

int
event::index_from_wait_result(
  wait_result result
  )
{
  return
    result == wait_result::timeout   ? -1 :
    result == wait_result::failed    ? -1 :
    result  > wait_result::abandoned ? (static_cast<int>(result) - static_cast<int>(wait_result::abandoned)) :
    static_cast<int>(result) - static_cast<int>(wait_result::success);
}

bool
event::is_signaled(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  return wait(0) == wait_result::success;
#else
  pthread_mutex_lock(&_mutex);
  bool result = _signaled;
  pthread_mutex_unlock(&_mutex);
  return result;
#endif
}

}
