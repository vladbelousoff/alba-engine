#pragma once

#include <functional>
#include <openssl/evp.h>

#include "engine/utils/types.h"
#include "libassert/assert.hpp"

namespace loki {

  struct GenericHashImpl
  {
    typedef EVP_MD const* (*HashCreator)();

    static auto create_context() noexcept -> EVP_MD_CTX*
    {
      return EVP_MD_CTX_new();
    }

    static void destroy_context(EVP_MD_CTX* ctx)
    {
      EVP_MD_CTX_free(ctx);
    }
  };

  template<GenericHashImpl::HashCreator HashCreator, size_t DigestLength>
  class GenericHash
  {
  public:
    static constexpr size_t DIGEST_LENGTH = DigestLength;
    using Digest = std::array<u8, DIGEST_LENGTH>;

    static auto get_digest_of(const u8* data, size_t len) -> Digest
    {
      GenericHash hash;
      hash.update_data(data, len);
      hash.finalize();
      return hash.get_digest();
    }

    template<typename... Ts>
    static auto get_digest_of(Ts&&... pack) -> std::enable_if_t<!(std::is_integral_v<std::decay_t<Ts>> || ...), Digest>
    {
      GenericHash hash;
      (hash.update_data(std::forward<Ts>(pack)), ...);
      hash.finalize();
      return hash.get_digest();
    }

    GenericHash()
      : ctx(GenericHashImpl::create_context())
    {
      i32 result = EVP_DigestInit_ex(ctx, HashCreator(), nullptr);
      DEBUG_ASSERT(result == 1);
    }

    GenericHash(const GenericHash& other)
      : ctx(GenericHashImpl::create_context())
    {
      *this = other;
    }

    GenericHash(GenericHash&& other) noexcept
    {
      *this = std::move(other);
    }

    ~GenericHash()
    {
      if (ctx) {
        GenericHashImpl::destroy_context(ctx);
        ctx = nullptr;
      }
    }

    GenericHash& operator=(const GenericHash& other)
    {
      if (this == &other) {
        return *this;
      }

      i32 result = EVP_MD_CTX_copy_ex(ctx, other.ctx);
      DEBUG_ASSERT(result == 1);
      digest = other.digest;

      return *this;
    }

    GenericHash& operator=(GenericHash&& other) noexcept
    {
      if (this == &other) {
        return *this;
      }

      ctx = std::exchange(other.ctx, GenericHashImpl::create_context());
      digest = std::exchange(other.digest, Digest{});

      return *this;
    }

    void update_data(const u8* data, size_t len)
    {
      i32 result = EVP_DigestUpdate(ctx, data, len);
      DEBUG_ASSERT(result == 1);
    }

    void update_data(std::string_view str)
    {
      update_data(reinterpret_cast<const u8*>(str.data()), str.size());
    }

    void update_data(const std::string& str)
    {
      update_data(std::string_view(str));
    }

    void update_data(const char* str)
    {
      update_data(std::string_view(str));
    }

    template<typename Container>
    void update_data(const Container& c)
    {
      update_data(std::data(c), std::size(c));
    }

    void finalize()
    {
      u32 length;
      i32 result = EVP_DigestFinal_ex(ctx, digest.data(), &length);
      DEBUG_ASSERT(result == 1);
      DEBUG_ASSERT(length == DIGEST_LENGTH);
    }

    const Digest& get_digest() const
    {
      return digest;
    }

  private:
    EVP_MD_CTX* ctx{};
    Digest digest{};
  };

  namespace crypto {

    namespace constants {
      static constexpr size_t MD5_DIGEST_LENGTH_BYTES = 16;
      static constexpr size_t SHA1_DIGEST_LENGTH_BYTES = 20;
      static constexpr size_t SHA256_DIGEST_LENGTH_BYTES = 32;
    }; // namespace constants

    using MD5 = GenericHash<EVP_md5, constants::MD5_DIGEST_LENGTH_BYTES>;
    using SHA1 = GenericHash<EVP_sha1, constants::SHA1_DIGEST_LENGTH_BYTES>;
    using SHA256 = GenericHash<EVP_sha256, constants::SHA256_DIGEST_LENGTH_BYTES>;

  } // namespace crypto

} // namespace loki
