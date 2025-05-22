#pragma once
#include <mini/common.h>

#include <functional>

namespace mini {

namespace detail {

template <
  typename T
>
size_type
hash_array_representation(
  const T* buffer,
  size_type size
  )
{
  size_type hash = 0;
  
  for (size_type i = 0; i < size; i++)
  {
    hash = hash * 31 + buffer[i];
  }
  
  return hash;
}

template <
  typename T,
  typename = void
>
struct hash_impl
{
  size_type
  operator()(
    const T& value
    ) const
  {
    return std::hash<T>()(value);
  }
};

template <
  typename T
>
struct hash_impl<
  T,
  typename std::enable_if<
    std::is_pointer<T>::value
  >::type
>
{
  size_type
  operator()(
    const T& value
    ) const
  {
    return reinterpret_cast<size_type>(value);
  }
};

template <
  typename T
>
struct hash_impl<
  T,
  typename std::enable_if<
    std::is_enum<T>::value
  >::type
>
{
  size_type
  operator()(
    const T& value
    ) const
  {
    return static_cast<size_type>(value);
  }
};

template <
  typename T
>
struct hash_impl<
  T*,
  typename std::enable_if<
    std::is_function<T>::value
  >::type
>
{
  size_type
  operator()(
    T* const& value
    ) const
  {
    return reinterpret_cast<size_type>(value);
  }
};

template <
  typename T
>
struct hash_impl<
  T* const&,
  typename std::enable_if<
    std::is_function<T>::value
  >::type
>
{
  size_type
  operator()(
    T* const& value
    ) const
  {
    return reinterpret_cast<size_type>(value);
  }
};

}

template <
  typename T
>
struct hash
{
  size_type
  operator()(
    const T& value
    ) const
  {
    return detail::hash_impl<T>()(value);
  }
};

}