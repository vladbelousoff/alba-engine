#pragma once

#include "byte_buffer.h"

#define LOKI_DECLARE_PACKET_FIELD(NAME, TYPE)       loki::PacketFieldT<TYPE, 0x01> NAME{ this };
#define LOKI_DECLARE_PACKET_ARRAY(NAME, TYPE, SIZE) loki::PacketFieldT<TYPE, SIZE> NAME{ this };

namespace loki {

  class Packet;
  class PacketField;

  class PacketField
  {
    friend class Packet;

  public:
    explicit PacketField(Packet* pkt)
        : pkt{ *pkt }
    {
      add_field_in_pkt();
    }

  protected:
    virtual void insert_to(ByteBuffer& buffer) const = 0;
    void add_field_in_pkt();

  protected:
    Packet& pkt;
  };

  template <typename Type, size_t Size> class PacketFieldT : public PacketField
  {
    using Self = PacketFieldT<Type, Size>;

  public:
    explicit PacketFieldT(Packet* pkt)
        : PacketField{ pkt }
        , data{}
    {
    }

    Self& operator<<(std::string_view view)
    {
      DEBUG_ASSERT(view.size() <= Size);
      std::memcpy(data.data(), view.data(), view.size());
      return *this;
    }

  protected:
    void insert_to(ByteBuffer& buffer) const override
    {
      buffer.append(data);
    }

  private:
    std::array<Type, Size> data;
  };

  template <typename Type> class PacketFieldT<Type, 1> : public PacketField
  {
    using Self = PacketFieldT<Type, 1>;

  public:
    explicit PacketFieldT(Packet* pkt)
        : PacketField{ pkt }
        , value{}
    {
    }

    explicit PacketFieldT(Packet* pkt, Type value)
        : PacketField{ pkt }
        , value{ value }
    {
    }

  public:
    template <typename IncomingType> Self& operator<<(IncomingType new_value)
    {
      value = static_cast<Type>(new_value);
      return *this;
    }

  protected:
    void insert_to(ByteBuffer& buffer) const override
    {
      buffer.append<Type>(value);
    }

  private:
    Type value;
  };

  class Packet
  {
    friend class PacketField;

  public:
#if 0
    template <typename Type> void operator<<(Type value)
    {
      fields.push_back(new PacketFieldT<Type, 1>{ this, value });
    }
#endif

    Packet& operator>>(PacketField& field);

    void finalize_buffer(loki::ByteBuffer& buffer) const;

  private:
    std::vector<PacketField*> fields{};
  };

} // namespace loki
