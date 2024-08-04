#include "byte_buffer.h"

loki::ByteBuffer::ByteBuffer()
{
  buffer.reserve(DEFAULT_SIZE);
}

void
loki::ByteBuffer::append(const std::vector<loki::u8>& value)
{
  append<loki::u8>(value.size());
  buffer.insert(buffer.end(), value.begin(), value.end());
}

void
loki::ByteBuffer::append(std::string_view value)
{
  buffer.insert(buffer.end(), value.begin(), value.end());
  buffer.push_back(0);
}

void
loki::ByteBuffer::send(sockpp::tcp_socket& conn) const
{
  conn.write(buffer.data(), buffer.size());
}

ssize_t
loki::ByteBuffer::recv(sockpp::tcp_socket& conn)
{
  buffer.resize(DEFAULT_SIZE);

  ssize_t n = conn.read(buffer.data(), DEFAULT_SIZE);

  if (n >= 0) {
    buffer.resize(n);
  }

  if (n < 0) {
    spdlog::error("Error: {}!", n);
  }

  return n;
}

void
loki::ByteBuffer::reset()
{
  r_pos = 0;
  buffer.clear();
}
