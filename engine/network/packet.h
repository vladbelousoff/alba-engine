#pragma once

#include <iomanip>

#include "byte_buffer.h"
#include "engine/utils/types.h"

#define LOKI_DECLARE_PACKET_FIELD(NAME, TYPE)       loki::PacketFieldT<TYPE, 0x01>(NAME){ #NAME, this };
#define LOKI_DECLARE_PACKET_ARRAY(NAME, TYPE, SIZE) loki::PacketFieldT<TYPE, SIZE>(NAME){ #NAME, this };
#define LOKI_DECLARE_PACKET_BLOCK(NAME)             loki::PacketFieldArray NAME{ #NAME, this };
#define LOKI_DECLARE_PACKET_STRING(NAME)            loki::PacketFieldString NAME{ #NAME, this };

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
  public:
    explicit PacketFieldT(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
      , data{}
    {
    }

    void set(std::string_view view)
    {
      DEBUG_ASSERT(view.size() <= Size);
      std::reverse_copy(view.begin(), view.end(), data.begin());
    }

    void set(const std::vector<u8>& view)
    {
      DEBUG_ASSERT(view.size() <= Size);
      std::copy(view.begin(), view.end(), data.begin());
    }

    void set(const std::array<u8, Size>& view)
    {
      DEBUG_ASSERT(view.size() <= Size);
      std::copy(view.begin(), view.end(), data.begin());
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

    std::array<Type, Size>& operator*()
    {
      return get();
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
    void set(Type new_value)
    {
      value = new_value;
    }

    Type& get()
    {
      return value;
    }

    Type& operator*()
    {
      return get();
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
  public:
    explicit PacketFieldArray(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
    {
    }

  public:
    void set(std::string_view view)
    {
      data.resize(view.size());
      std::memcpy(data.data(), view.data(), view.size());
    }

    const std::vector<loki::u8>& get()
    {
      return data;
    }

    const std::vector<loki::u8>& operator*()
    {
      return get();
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

  class PacketFieldString : public PacketField
  {
  public:
    explicit PacketFieldString(std::string_view name, Packet* pkt)
      : PacketField{ name, pkt }
    {
    }

  public:
    void set(std::string_view view)
    {
      data.resize(view.size());
      std::memcpy(data.data(), view.data(), view.size());
    }

    const std::string& get()
    {
      return data;
    }

    const std::string& operator*()
    {
      return get();
    }

    std::string to_string() const override
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
      std::ostringstream oss;
      char ch;
      do {
        ch = buffer.read<char>();
        oss << ch;
      } while (ch != 0);

      data = oss.str();
    }

    std::string data;
  };

  class Packet
  {
    friend class PacketField;

  public:
    Packet() = default;
    
    Packet(const Packet& other) = delete;
    Packet& operator=(const Packet& other) = delete;

  public:
    void save_buffer(loki::ByteBuffer& buffer) const;
    void load_buffer(loki::ByteBuffer& buffer);

    void for_each_field(const std::function<void(const PacketField&)>& cb) const;

  private:
    std::vector<PacketField*> fields{};
  };

} // namespace loki
