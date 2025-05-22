#pragma once
#include "common.h"
#include "capi/hash.h"
#ifdef MINI_OS_WINDOWS
#include "cng/hash.h"
#endif

namespace mini::crypto {

using sha1 = MINI_CRYPTO_HASH_NAMESPACE::hash<hash_algorithm_type::sha1>;

}
