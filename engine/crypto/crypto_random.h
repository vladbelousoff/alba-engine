#pragma once

#include <array>

#include "engine/utils/types.h"

namespace loki::crypto {

  void get_random_bytes(u8* buf, size_t len);

  template<typename Container>
  void get_random_bytes(Container& c)
  {
    get_random_bytes(std::data(c), std::size(c));
  }

  template<size_t S>
  std::array<u8, S> get_random_bytes()
  {
    std::array<u8, S> arr;
    get_random_bytes(arr);
    return arr;
  }

} // namespace loki::crypto
