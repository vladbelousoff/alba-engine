#pragma once

#include <string>
#include <vector>

#include "engine/utils/endianness.h"
#include "libassert/assert.hpp"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"
#include "spdlog/spdlog.h"

namespace loki {

  class ByteBuffer
  {
    constexpr static size_t DEFAULT_SIZE = 0x1000;

  public:
    explicit ByteBuffer()
        : ByteBuffer(Endianness::LittleEndian)
    {
    }

    explicit ByteBuffer(Endianness endianness);

  public:
    template <typename T, typename U> void append(U value)
    {
      const T converted_value = loki::to_endianness(static_cast<T>(value), endianness);
      auto bytes = reinterpret_cast<const std::uint8_t*>(&converted_value);
      buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
    }

    void append(std::string_view value);

    template <typename Type, size_t Size> void append(const std::array<Type, Size>& value)
    {
      buffer.insert(buffer.end(), value.rbegin(), value.rend());
    }

    template <typename T> T read()
    {
      T value = read<T>(read_pos);
      read_pos += sizeof(T);
      return value;
    }

    template <size_t Size, typename T> void read(std::array<T, Size>& arr)
    {
      read_bytes(arr.data(), Size * sizeof(T));
    }

    void read_bytes(char* data, std::size_t n)
    {
      DEBUG_ASSERT(read_pos + n <= buffer.size());
      std::memcpy(data, &buffer[read_pos], n);
      read_pos += n;
    }

    void send(sockpp::tcp_connector& conn) const;
    void receive(sockpp::tcp_connector& conn);

  protected:
    template <typename T> T read(std::size_t from_pos) const
    {
      DEBUG_ASSERT(from_pos + sizeof(T) <= buffer.size());
      T value = *((const T*)&buffer[from_pos]);
      const T converted_value = loki::to_endianness(value, endianness);
      return converted_value;
    }

  protected:
    Endianness endianness;
    std::vector<std::uint8_t> buffer;
    std::size_t read_pos = 0;
  };

} // namespace loki

