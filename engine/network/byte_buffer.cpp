#include "byte_buffer.h"

loki::ByteBuffer::ByteBuffer(Endianness endianness)
    : endianness{ endianness }
{
}

void loki::ByteBuffer::write(const std::string& string, bool reversed)
{
  if (reversed) {
    buffer.insert(buffer.end(), string.rbegin(), string.rend());
  } else {
    buffer.insert(buffer.end(), string.begin(), string.end());
  }
}

void loki::ByteBuffer::send(sockpp::tcp_connector& conn) const
{
  conn.write(buffer.data(), buffer.size());
}
