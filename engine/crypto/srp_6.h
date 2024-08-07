#pragma once

#include <array>
#include <string_view>
#include <vector>

#include "engine/crypto/crypto_hash.h"
#include "engine/network/auth_defines.h"
#include "engine/utils/big_num.h"
#include "engine/utils/types.h"

namespace loki {

  using namespace crypto;

  class SRP6
  {
  public:
    static constexpr size_t SALT_LENGTH = 32;
    using Salt = std::array<u8, SALT_LENGTH>;
    static constexpr size_t EPHEMERAL_KEY_LENGTH = 32;
    using EphemeralKey = std::array<u8, EPHEMERAL_KEY_LENGTH>;

  public:
    explicit SRP6(const BigNum& N, const BigNum& g);
    ~SRP6() = default;

  public:
    void generate(const Salt& salt, const EphemeralKey& B, std::string_view I, std::string_view P);

    const EphemeralKey& get_A() const
    {
      return A;
    }

    const SHA1::Digest& get_client_M() const
    {
      return client_M;
    }

    const SHA1::Digest& get_crc_hash() const
    {
      return crc_hash;
    }

    const SessionKey& get_session_key() const
    {
      return K;
    }

  private:
    static SessionKey SHA1_interleave(const EphemeralKey& S);

  private:
    BigNum N;
    BigNum g;
    BigNum a;
    BigNum k;
    EphemeralKey A;
    SHA1::Digest client_M{};
    SHA1::Digest crc_hash{};
    SessionKey K{};
  };

} // namespace loki

