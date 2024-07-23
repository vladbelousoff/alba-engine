#include "packet.h"

loki::Packet::Packet(Endianness endianness)
    : endianness{ endianness }
{
}

void loki::Packet::write(const std::string& string, StringRepr repr)
{
  switch (repr) {
    case StringRepr::NORMAL:
      buffer.insert(buffer.end(), string.begin(), string.end());
      break;
    case StringRepr::REVERSED:
      buffer.insert(buffer.end(), string.rbegin(), string.rend());
      break;
  }
}

void loki::Packet::send(sockpp::tcp_connector& conn) const
{
  conn.write(buffer.data(), buffer.size());
}
