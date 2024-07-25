#pragma once

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/srp.h>

#include <array>
#include <string_view>
#include <vector>

#include "engine/utils/types.h"

namespace loki {

  class SRP
  {
  public:
    explicit SRP(const std::vector<loki::u8>& in_N, loki::u8 in_g);
    ~SRP();

  public:
    void generate(const std::array<loki::u8, 32>& salt, const std::array<loki::u8, 32>& in_B, std::string_view username, std::string_view password);

  private:
    static void hash_sha1(const std::vector<u8>& input, std::array<u8, SHA_DIGEST_LENGTH>& output);

  private:
    BIGNUM* N = nullptr;
    BIGNUM* g = nullptr;
    BIGNUM* A = nullptr;
    BIGNUM* B = nullptr;
    BIGNUM* a = nullptr;
    BIGNUM* u = nullptr;
    BIGNUM* x = nullptr;
    BIGNUM* v = nullptr;
    BIGNUM* S = nullptr;
    std::array<u8, SHA_DIGEST_LENGTH> M1{};
  };

} // namespace loki

