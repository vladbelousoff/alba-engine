#pragma once

#include "engine/utils/types.h"
#include <array>

namespace loki {

  static constexpr size_t SESSION_KEY_LENGTH = 40;
  using SessionKey = std::array<u8, SESSION_KEY_LENGTH>;

} // namespace loki
