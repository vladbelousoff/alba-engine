#pragma once

#include <array>

#include "engine/utils/types.h"
#include "openssl/evp.h"

namespace loki {

  class ARC4
  {
  public:
    ARC4();
    ~ARC4();

  public:
    void init(const u8* seed, size_t len);
    void update_data(u8* data, size_t len);

    template<typename Container>
    void init(const Container& c)
    {
      init(std::data(c), std::size(c));
    }

    template<typename Container>
    void update_data(Container& c)
    {
      update_data(std::data(c), std::size(c));
    }

  private:
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_CIPHER* cipher{};
#endif
    EVP_CIPHER_CTX* context{};
  };

} // namespace loki
