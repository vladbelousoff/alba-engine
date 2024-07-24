#include "packet.h"

loki::Packet& loki::Packet::operator>>(loki::PacketField& field)
{
  return *this;
}

void loki::Packet::finalize_buffer(loki::ByteBuffer& buffer) const
{
  for (auto* field : fields) {
    field->insert_to(buffer);
  }
}

void loki::PacketField::add_field_in_pkt()
{
  pkt.fields.push_back(this);
}
