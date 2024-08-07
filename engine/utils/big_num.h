#pragma once

#include <openssl/bn.h>
#include <stdexcept>
#include <vector>

#include "types.h"

namespace loki {

  class BigNum
  {
  public:
    BigNum();

    BigNum(const BigNum& other);
    BigNum& operator=(const BigNum& other);

    ~BigNum();

  public:
    void set_dword(i32 word);
    void set_dword(u32 word);
    void set_qword(u64 word);

    void set_binary(const u8* bytes, size_t len);

    template<typename Container>
    auto set_binary(const Container& c) -> std::enable_if_t<!std::is_pointer_v<std::decay_t<Container>>>
    {
      set_binary(std::data(c), std::size(c));
    }

    static BigNum from_binary(const u8* bytes, size_t len)
    {
      BigNum ret;
      ret.set_binary(bytes, len);
      return ret;
    }

    template<typename Container>
    static BigNum from_binary(const Container& c)
    {
      BigNum ret;
      ret.set_binary(c);
      return ret;
    }

    static BigNum from_random(i32 num_bits)
    {
      BigNum ret;
      ret.set_rand(num_bits);
      return ret;
    }

    bool set_hex_str(const char* str);
    void set_rand(i32 num_bits);

    BigNum exp(const BigNum& bn1) const;
    BigNum mod_exp(const BigNum& bn1, const BigNum& bn2) const;

    BigNum& operator+=(const BigNum& other);
    BigNum operator+(const BigNum& other) const;

    BigNum& operator-=(const BigNum& other);
    BigNum operator-(const BigNum& other) const;

    BigNum& operator*=(const BigNum& other);
    BigNum operator*(const BigNum& other) const;

    BigNum& operator/=(const BigNum& other);
    BigNum operator/(const BigNum& other) const;

    BigNum& operator%=(const BigNum& other);
    BigNum operator%(const BigNum& other) const;

    BigNum& operator<<=(int n);
    BigNum operator<<(int n) const;

    int cmp(const BigNum& other) const;

    bool operator==(const BigNum& other) const
    {
      return cmp(other) == 0;
    }

    int get_num_bytes() const
    {
      return BN_num_bytes(bn);
    }

    void get_bytes(u8* buf, size_t buf_size) const;

    std::vector<u8> to_byte_vector() const;

    template<std::size_t Size>
    std::array<u8, Size> to_byte_array() const
    {
      std::array<u8, Size> buf;
      get_bytes(buf.data(), Size);
      return buf;
    }

    std::string as_hex_str() const;

    BIGNUM* get() const
    {
      return bn;
    }

  private:
    BIGNUM* bn{};
  };

} // namespace loki

