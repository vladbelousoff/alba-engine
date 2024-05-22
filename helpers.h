#pragma once

namespace kiwi {

  /// Compile-time bit size counting
  template <int N> struct bits_count_helper
  {
    enum
    {
      VALUE = 1 + bits_count_helper<(N + 1) / 2>::VALUE
    };
  };

  template <int N> constexpr int bits_count_helper_v()
  {
    return bits_count_helper<N>::VALUE;
  }

  template <> struct bits_count_helper<1>
  {
    enum
    {
      VALUE = 0
    };
  };

  template <> struct bits_count_helper<0>
  {
    enum
    {
      VALUE = 0
    };
  };

} // namespace kiwi
