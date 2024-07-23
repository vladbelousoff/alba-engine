#pragma once

namespace loki {

  enum class Endianness
  {
    LittleEndian,
    BigEndian,
  };

  inline bool is_big_endian()
  {
    int i = 1;
    return !*((char*)&i);
  }

  // from http://stackoverflow.com/a/4956493/238609
  template <typename T> T swap_endian(T u)
  {
    union {
      T u;
      unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++) {
      dest.u8[k] = source.u8[sizeof(T) - k - 1];
    }

    return dest.u;
  }

  template <typename T> T to_big_endian(T u)
  {
    if (is_big_endian()) {
      return u;
    } else {
      return swap_endian<T>(u);
    }
  }

  template <typename T> T to_little_endian(T u)
  {
    if (!is_big_endian()) {
      return u;
    } else {
      return swap_endian<T>(u);
    }
  }

  template <typename T> T to_endianness(T u, Endianness endianness)
  {
    switch (endianness) {
      case Endianness::LittleEndian:
        return to_little_endian(u);
      case Endianness::BigEndian:
        return to_big_endian(u);
    }
  }

} // namespace loki
