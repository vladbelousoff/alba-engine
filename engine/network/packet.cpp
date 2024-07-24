#include "packet.h"

void loki::Packet::fill_buffer(loki::ByteBuffer& buffer) const
{
  for (auto* field : fields) {
    field->insert_to(buffer);
  }
}

void loki::Packet::parse_buffer(loki::ByteBuffer& buffer)
{
  for (auto* field : fields) {
    field->parse_from(buffer);
  }
}

void loki::Packet::operator>>(loki::ByteBuffer& buffer) const
{
  fill_buffer(buffer);
}

void loki::Packet::operator<<(loki::ByteBuffer& buffer)
{
  parse_buffer(buffer);
}

void loki::PacketField::add_field_in_pkt()
{
  pkt.fields.push_back(this);
}
