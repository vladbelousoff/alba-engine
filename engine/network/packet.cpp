#include "packet.h"

void loki::Packet::finalize_buffer(loki::ByteBuffer& buffer) const
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

void loki::PacketField::add_field_in_pkt()
{
  pkt.fields.push_back(this);
}
