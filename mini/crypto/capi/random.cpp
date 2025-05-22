#include "random.h"

#ifdef MINI_OS_WINDOWS
#include "provider.h"
#endif

namespace mini::crypto::capi {

byte_buffer
random::get_random_bytes(
  size_type byte_count
  )
{
  byte_buffer result(byte_count);
  get_random_bytes(result);

  return result;
}

void
random::get_random_bytes(
  mutable_byte_buffer_ref output
  )
{
#ifdef MINI_OS_WINDOWS
  CryptGenRandom(
    provider_factory.get_rsa_aes_handle(),
    static_cast<DWORD>(output.get_size()),
    output.get_buffer());
#else
  RAND_bytes(output.get_buffer(), static_cast<int>(output.get_size()));
#endif
}

random random_device;

}
