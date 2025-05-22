#include "hmac.h"

#ifdef MINI_OS_WINDOWS
#include "provider.h"
#endif

namespace mini::crypto::capi {

//
// constructors.
//

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>::hmac(
  const byte_buffer_ref key
  )
#ifdef MINI_OS_WINDOWS
  : HASH_TYPE(detail::hash_algorithm_type_map[static_cast<int>(hash_algorithm)], key, _key_handle)
#else
  : HASH_TYPE(detail::hash_algorithm_type_map[static_cast<int>(hash_algorithm)], key)
  , _key(key)
#endif
{

}

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>::hmac(
  const hmac& other
  )
{
  duplicate_internal(other);
}

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>::hmac(
  hmac&& other
  )
{
  swap(other);
}

//
// destructor.
//

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>::~hmac(
  void
  )
{
  destroy();
}

//
// assign operators.
//

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>&
hmac<HASH_TYPE>::operator=(
  const hmac& other
  )
{
  duplicate_internal(other);
  return *this;
}

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>&
hmac<HASH_TYPE>::operator=(
  hmac&& other
  )
{
  swap(other);
  return *this;
}

template <
  typename HASH_TYPE
>
hmac<HASH_TYPE>
hmac<HASH_TYPE>::duplicate(
  void
  )
{
  return hmac<HASH_TYPE>(*this);
}

//
// swap.
//

template <
  typename HASH_TYPE
>
void
hmac<HASH_TYPE>::swap(
  hmac& other
  )
{
  HASH_TYPE::swap(other);
#ifdef MINI_OS_WINDOWS
  mini::swap(_key_handle, other._key_handle);
#else
  mini::swap(_key, other._key);
#endif
}

//
// operations.
//

template <
  typename HASH_TYPE
>
byte_buffer
hmac<HASH_TYPE>::compute(
  const byte_buffer_ref key,
  const byte_buffer_ref input
  )
{
  hmac<HASH_TYPE> md(key);
  md.update(input);
  return md.get();
}

template <
  typename HASH_TYPE
>
void
hmac<HASH_TYPE>::destroy(
  void
  )
{
  HASH_TYPE::destroy();

#ifdef MINI_OS_WINDOWS
  if (_key_handle)
  {
    CryptDestroyKey(_key_handle);
    _key_handle = 0;
  }
#else
  _key.clear();
#endif
}


}