#include "byte_buffer.h"

loki::ByteBuffer::ByteBuffer(Endianness endianness)
    : endianness{ endianness }
{
  buffer.reserve(DEFAULT_SIZE);
}

void loki::ByteBuffer::append(std::string_view value)
{
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
    spdlog::info("Received {} bytes", n);
    buffer.resize(n);
  } else {
    spdlog::error("Error receiving!");
  }
}
