#pragma once
#include <mini/byte_buffer.h>

#include <type_traits>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#include <wincrypt.h>
#else
#include <openssl/rand.h>
#endif

namespace mini::crypto::capi {

class random
{
  MINI_MAKE_NONCOPYABLE(random);

  public:
    random(
      void
      ) = default;

    byte_buffer
    get_random_bytes(
      size_type byte_count
      );

    void
    get_random_bytes(
      mutable_byte_buffer_ref output
      );

    template <
      typename T,
      typename = std::enable_if_t<std::is_integral_v<T>>
    >
    T
    get_random(
      void
      )
    {
      T result;
      get_random_bytes(mutable_byte_buffer_ref(
        reinterpret_cast<byte_type*>(&result),
        reinterpret_cast<byte_type*>(&result) + sizeof(T)));

      return result;
    }

    template <
      typename T,
      typename = std::enable_if_t<std::is_integral_v<T>>
    >
    T
    get_random(
        T max
      )
    {
      return get_random<std::make_unsigned_t<T>>() % max;
    }
};

extern random random_device;

}
