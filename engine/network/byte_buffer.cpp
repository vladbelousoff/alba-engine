#include "byte_buffer.h"

loki::ByteBuffer::ByteBuffer(Endianness endianness)
    : endianness{ endianness }
{
  buffer.reserve(DEFAULT_SIZE);
}

void loki::ByteBuffer::append(std::string_view value)
{
  append<loki::u8>(value.length());
  buffer.insert(buffer.end(), value.begin(), value.end());
}

void loki::ByteBuffer::append(const std::vector<loki::u8>& value)
{
  append<loki::u8>(value.size());
  buffer.insert(buffer.end(), value.begin(), value.end());
}

void loki::ByteBuffer::send(sockpp::tcp_connector& conn) const
{
  conn.write(buffer.data(), buffer.size());
}

void loki::ByteBuffer::receive(sockpp::tcp_connector& conn)
{
  buffer.resize(DEFAULT_SIZE);

  ssize_t n = conn.read(buffer.data(), DEFAULT_SIZE);
  
  if (n >= 0) {
    buffer.resize(n);
  }

  if (n <= 0) {
    spdlog::error("Error: !", n);
  }
}

void loki::ByteBuffer::reset()
{
  read_pos = 0;
  buffer.clear();
}
