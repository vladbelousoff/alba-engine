#include "packet.h"

loki::Packet& loki::Packet::operator<<(const loki::PacketField& field)
{
  return *this;
}

loki::Packet& loki::Packet::operator>>(loki::PacketField& field)
{
  return *this;
}

loki::Packet::Packet(loki::Endianness endianness)
    : ByteBuffer(endianness)
{
}

