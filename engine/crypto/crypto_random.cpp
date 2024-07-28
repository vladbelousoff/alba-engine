#pragma once

#include "crypto_random.h"
#include "libassert/assert.hpp"

#include <openssl/rand.h>

void
loki::crypto::get_random_bytes(loki::u8* buf, size_t len)
{
  i32 result = RAND_bytes(buf, static_cast<i32>(len));
  DEBUG_ASSERT(result == 1, "Not enough randomness in OpenSSL's entropy pool. What in the world are you running on?");
}
