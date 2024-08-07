#include "arc_4.h"
#include "engine/utils/types.h"
#include "libassert/assert.hpp"

loki::ARC4::ARC4()
  : context(EVP_CIPHER_CTX_new())
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  cipher = EVP_CIPHER_fetch(nullptr, SN_rc4, nullptr);
#else
  const EVP_CIPHER* cipher = EVP_rc4();
#endif

  EVP_CIPHER_CTX_init(context);
  i32 result = EVP_EncryptInit_ex(context, cipher, nullptr, nullptr, nullptr);
  ASSERT(result == 1);
}

loki::ARC4::~ARC4()
{
  EVP_CIPHER_CTX_free(context);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  EVP_CIPHER_free(cipher);
#endif
}

void
loki::ARC4::init(const loki::u8* seed, size_t len)
{
  i32 result1 = EVP_CIPHER_CTX_set_key_length(context, (int)len);
  ASSERT(result1 == 1);
  i32 result2 = EVP_EncryptInit_ex(context, nullptr, nullptr, seed, nullptr);
  ASSERT(result2 == 1);
}

void
loki::ARC4::update_data(loki::u8* data, size_t len)
{
  i32 outlen = 0;
  i32 result1 = EVP_EncryptUpdate(context, data, &outlen, data, (int)len);
  ASSERT(result1 == 1);
  i32 result2 = EVP_EncryptFinal_ex(context, data, &outlen);
  ASSERT(result2 == 1);
}
