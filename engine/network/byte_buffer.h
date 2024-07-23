#pragma once

#include <string>
#include <vector>

#include "engine/utils/endianness.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"
#include "spdlog/spdlog.h"

namespace loki {

  class ByteBuffer
  {
  public:
    explicit ByteBuffer(Endianness endianness);

  public:
    template <typename T, typename U> void write(U value)
    {
      const T big_endian_value = loki::to_endianness(static_cast<T>(value), endianness);
      auto bytes = reinterpret_cast<const std::uint8_t*>(&big_endian_value);
      buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
    }

    void write(const std::string& string, bool reversed = true);
    void send(sockpp::tcp_connector& conn) const;

  private:
    Endianness endianness;
    std::vector<std::uint8_t> buffer;
  };

} // namespace loki

