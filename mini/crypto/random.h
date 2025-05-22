#pragma once
#include "common.h"
#include "capi/random.h"
#ifdef MINI_OS_WINDOWS
#include "cng/random.h"
#endif

namespace mini::crypto {

using random = MINI_CRYPTO_RANDOM_NAMESPACE::random;

//
// TODO:
// it would be great to come up with better
// solution than this.
//
static auto& random_device = MINI_CRYPTO_RANDOM_NAMESPACE::random_device;

}
