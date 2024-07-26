#pragma once

#include <iomanip>

#include "byte_buffer.h"
#include "engine/utils/types.h"

#define LOKI_DECLARE_PACKET_FIELD(NAME, TYPE)       loki::PacketFieldT<TYPE, 0x01>(NAME){ #NAME, this };
#define LOKI_DECLARE_PACKET_ARRAY(NAME, TYPE, SIZE) loki::PacketFieldT<TYPE, SIZE>(NAME){ #NAME, this };
#define LOKI_DECLARE_PACKET_BLOCK(NAME)             loki::PacketFieldArray NAME{ #NAME, this };

namespace loki {

  class Packet;
  class PacketField;

  class PacketField
  {
    friend class Packet;

  public:
    explicit PacketField(std::string_view name, Packet* pkt)
      : name{ name }
      , pkt{ *pkt }
    {
      add_field_in_pkt();
    }

  public:
    virtual std::string to_string() const = 0;

    std::string get_name() const
    {
      return name;
    }

  protected:
    virtual void save(ByteBuffer& buffer) const = 0;
    virtual void load(ByteBuffer& buffer) = 0;

    void add_field_in_pkt();

  protected:
    std::string name;
    Packet& pkt;
  };

  template<typename Type, size_t Size>
  class PacketFieldT : public PacketField
  {
    using Self = PacketFieldT<Type, Size>;

  public:
    explicit PacketFieldT(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
      , data{}
    {
    }

    Self& operator<<(std::string_view view)
    {
      DEBUG_ASSERT(view.size() <= Size);
      std::reverse_copy(view.begin(), view.end(), data.begin());
      return *this;
    }

  public:
    std::string to_string() const override
    {
      std::ostringstream oss;
      for (const auto& byte : data) {
        oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
      }
      return oss.str();
    }

    std::array<Type, Size>& get()
    {
      return data;
    }

  protected:
    void save(ByteBuffer& buffer) const override
    {
      buffer.append(data);
    }

    void load(ByteBuffer& buffer) override
    {
      buffer.read(data.data(), data.size());
    }

  private:
    std::array<Type, Size> data;
  };

  template<typename Type>
  class PacketFieldT<Type, 1> : public PacketField
  {
    using Self = PacketFieldT<Type, 1>;

  public:
    explicit PacketFieldT(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
      , value{}
    {
    }

    explicit PacketFieldT(std::string_view name, Packet* pkt, Type value)
      : PacketField{ name, pkt }
      , value{ value }
    {
    }

  public:
    std::string to_string() const override
    {
      return std::to_string(value);
    }

  public:
    template<typename IncomingType>
    Self& operator<<(IncomingType new_value)
    {
      value = static_cast<Type>(new_value);
      return *this;
    }

    Type get() const
    {
      return value;
    }

  protected:
    void save(ByteBuffer& buffer) const override
    {
      buffer.append<Type>(value);
    }

    void load(ByteBuffer& buffer) override
    {
      buffer.read(&value, sizeof(value));
    }

  private:
    Type value;
  };

  class PacketFieldArray : public PacketField
  {
    using Self = PacketFieldArray;

  public:
    explicit PacketFieldArray(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
    {
    }

  public:
    Self& operator<<(std::string_view view)
    {
      data.resize(view.size());
      std::memcpy(data.data(), view.data(), view.size());
      return *this;
    }

    const std::vector<loki::u8>& get()
    {
      return data;
    }

    std::string to_string() const override
    {
      std::ostringstream oss;
      for (const auto& byte : data) {
        oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
      }
      return oss.str();
    }

  protected:
    void save(ByteBuffer& buffer) const override
    {
      buffer.append(data);
    }

    void load(ByteBuffer& buffer) override
    {
      data.resize(buffer.read<loki::u8>());
      buffer.read(data.data(), data.size());
    }

    std::vector<loki::u8> data;
  };

  class Packet
  {
    friend class PacketField;

  public:
    void operator>>(loki::ByteBuffer& buffer) const;
    void operator<<(loki::ByteBuffer& buffer);

    void for_each_field(const std::function<void(const PacketField&)>& cb) const;

  protected:
    void save_buffer(loki::ByteBuffer& buffer) const;
    void load_buffer(loki::ByteBuffer& buffer);

  private:
    std::vector<PacketField*> fields{};
  };

} // namespace loki
