#pragma once

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/srp.h>

#include <array>
#include <string_view>
#include <vector>

#include "engine/crypto/crypto_hash.h"
#include "engine/utils/big_num.h"
#include "engine/utils/types.h"

namespace loki {

  using namespace crypto;

  class SRP6
  {
  public:
    static constexpr size_t SALT_LENGTH = 32;
    using Salt = std::array<u8, SALT_LENGTH>;
    static constexpr size_t VERIFIER_LENGTH = 32;
    using Verifier = std::array<u8, VERIFIER_LENGTH>;
    static constexpr size_t EPHEMERAL_KEY_LENGTH = 32;
    using EphemeralKey = std::array<u8, EPHEMERAL_KEY_LENGTH>;
    static constexpr size_t SESSION_KEY_LENGTH = 40;
    using SessionKey = std::array<u8, SESSION_KEY_LENGTH>;

  public:
    explicit SRP6(const BigNum& N, const BigNum& g);
    ~SRP6() = default;

  public:
    std::tuple<loki::SRP6::SessionKey, loki::SHA1::Digest, loki::SHA1::Digest> generate(const Salt& salt, const EphemeralKey& B, std::string_view I, std::string_view P);

    const EphemeralKey& get_A() const
    {
      return A;
    }

  private:
    static SessionKey SHA1_interleave(const EphemeralKey& S);

  private:
    BigNum N;
    BigNum g;
    BigNum a;
    BigNum k;
    EphemeralKey A;
  };

} // namespace loki

