#include "packet.h"

void
loki::Packet::save_buffer(loki::ByteBuffer& buffer) const
{
  for (auto* field : fields) {
    field->save(buffer);
  }
}

void
loki::Packet::load_buffer(loki::ByteBuffer& buffer)
{
  for (auto* field : fields) {
    field->load(buffer);
  }
}

void
loki::Packet::for_each_field(const std::function<void(const PacketField&)>& cb) const
{
  for (auto* field : fields) {
    cb(*field);
  }
}

void
loki::PacketField::add_field_in_pkt()
{
  pkt.fields.push_back(this);
}
