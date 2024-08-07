#pragma once

#include "auth_defines.h"
#include "engine/crypto/arc_4.h"

namespace loki {

  class AuthCrypt
  {
  public:
    void init(const SessionKey& session_key);
    void decrypt_recv(u8* data, size_t len);
    void encrypt_send(u8* data, size_t len);

    bool is_inited() const
    {
      return initialized;
    }

  private:
    ARC4 decrypt{};
    ARC4 encrypt{};
    bool initialized = false;
  };

} // namespace loki
