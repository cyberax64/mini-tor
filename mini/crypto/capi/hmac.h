#pragma once
#include "hash.h"

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#include <wincrypt.h>
#else
#include <openssl/hmac.h>
#endif

namespace mini::crypto::capi {

template <
  typename HASH_TYPE
>
class hmac
  : public HASH_TYPE
{
  public:
    static constexpr size_type hash_size          = HASH_TYPE::hash_size;
    static constexpr size_type hash_size_in_bytes = HASH_TYPE::hash_size_in_bytes;
    static constexpr hash_algorithm_type hash_algorithm = HASH_TYPE::hash_algorithm;

    //
    // constructors.
    //

    hmac(
      const byte_buffer_ref key
      );

    hmac(
      const hmac& other
      );

    hmac(
      hmac&& other
      );

    //
    // destructor.
    //

    ~hmac(
      void
      );

    //
    // assign operators.
    //

    hmac&
    operator=(
      const hmac& other
      );

    hmac&
    operator=(
      hmac&& other
      );

    hmac
    duplicate(
      void
      );

    //
    // swap.
    //

    void
    swap(
      hmac& other
      );

    //
    // operations.
    //

    static byte_buffer
    compute(
      const byte_buffer_ref key,
      const byte_buffer_ref input
      );

  private:
    void
    destroy(
      void
      );

#ifdef MINI_OS_WINDOWS
    HCRYPTKEY _key_handle = 0;
#else
    byte_buffer _key;
#endif
};

}

#include "hmac.inl"
