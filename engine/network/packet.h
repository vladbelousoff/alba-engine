#pragma once

#include "byte_buffer.h"

#define LOKI_GET_ARG_COUNT_PRIVATE(...)                           LOKI_GET_ARG_COUNT_PRIVATE_HELPER(__VA_ARGS__, 3, 2, 1)
#define LOKI_GET_ARG_COUNT_PRIVATE_HELPER(_1, _2, _3, COUNT, ...) COUNT

#define LOKI_DECLARE_PACKET_FIELD_2(NAME, TYPE)                   loki::PacketFieldT<TYPE, 1> NAME{ this };
#define LOKI_DECLARE_PACKET_FIELD_3(NAME, TYPE, SIZE)             loki::PacketFieldT<TYPE, SIZE> NAME{ this };
#define LOKI_DECLARE_PACKET_FIELD_CHOOSER(ARG_COUNT, ...)         LOKI_DECLARE_PACKET_FIELD_CHOOSER_HELPER(ARG_COUNT, __VA_ARGS__)
#define LOKI_DECLARE_PACKET_FIELD_CHOOSER_HELPER(ARG_COUNT, ...)  LOKI_DECLARE_PACKET_FIELD_##ARG_COUNT(__VA_ARGS__)
#define LOKI_DECLARE_PACKET_FIELD(...)                            LOKI_DECLARE_PACKET_FIELD_CHOOSER(LOKI_GET_ARG_COUNT_PRIVATE(__VA_ARGS__), __VA_ARGS__)

namespace loki {

  class PacketField;

  class Packet : public ByteBuffer
  {
    friend class PacketField;

  public:
    explicit Packet(Endianness endianness);

  public:
    Packet& operator<<(const PacketField& field);
    Packet& operator>>(PacketField& field);

  private:
    std::vector<PacketField*> fields{};
  };

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
    void add_field_in_pkt()
    {
      pkt.fields.push_back(this);
    }

  protected:
    Packet& pkt;
  };

  template <typename Type, size_t Size> class PacketFieldT : public PacketField
  {
  public:
    explicit PacketFieldT(Packet* pkt)
        : PacketField{ pkt }
        , data{}
    {
    }

  private:
    std::array<Type, Size> data;
  };

  template <typename Type> class PacketFieldT<Type, 1> : public PacketField
  {
  public:
    explicit PacketFieldT(Packet* pkt)
        : PacketField{ pkt }
        , value{}
    {
    }

  private:
    Type value;
  };

} // namespace loki
