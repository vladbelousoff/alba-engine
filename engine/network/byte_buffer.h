#pragma once

#include <string>
#include <vector>

#include "engine/utils/types.h"
#include "libassert/assert.hpp"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"
#include "spdlog/spdlog.h"

namespace loki {

  class ByteBuffer
  {
    constexpr static size_t DEFAULT_SIZE = 0x1000;

  public:
    explicit ByteBuffer();

  public:
    void reset();

    template<typename T, typename U>
    void append(U value)
    {
      const T converted_value = static_cast<T>(value);
      auto bytes = reinterpret_cast<const std::uint8_t*>(&converted_value);
      buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
    }

    void append(const std::vector<loki::u8>& value);
    void append(std::string_view value);

    template<typename Type, size_t Size>
    void append(const std::array<Type, Size>& value)
    {
      buffer.insert(buffer.end(), value.begin(), value.end());
    }

    template<typename T>
    T read()
    {
      T value;
      read(&value, sizeof(T));
      return value;
    }

    template<typename Type, size_t Size>
    void read(std::array<Type, Size>& arr)
    {
      read(arr.data(), Size * sizeof(Type));
    }

    void read(void* data, std::size_t n)
    {
      DEBUG_ASSERT(r_pos + n <= buffer.size());
      std::memcpy(data, &buffer[r_pos], n);
      r_pos += n;
    }

    void send(sockpp::tcp_socket& conn) const;
    void recv(sockpp::tcp_socket& conn);

  protected:
    std::vector<loki::u8> buffer;
    std::size_t r_pos = 0;
  };

} // namespace loki

