#include "big_num.h"

loki::BigNum::BigNum()
  : bn(BN_new())
{
  if (!bn) {
    throw std::runtime_error("Failed to create BIGNUM");
  }
}

loki::BigNum::BigNum(const loki::BigNum& other)
  : bn(BN_dup(other.bn))
{
}

loki::BigNum&
loki::BigNum::operator=(const loki::BigNum& other)
{
  if (this != &other) {
    if (!BN_copy(bn, other.bn)) {
      throw std::runtime_error("Failed to copy BIGNUM");
    }
  }
  return *this;
}

loki::BigNum::~BigNum()
{
  BN_free(bn);
}

void
loki::BigNum::set_dword(loki::i32 word)
{
  set_dword(static_cast<u32>(abs(word)));
  if (word < 0) {
    BN_set_negative(bn, 1);
  }
}

void
loki::BigNum::set_dword(loki::u32 word)
{
  BN_set_word(bn, word);
}

void
loki::BigNum::set_qword(loki::u64 word)
{
  BN_set_word(bn, (u32)(word >> 32));
  BN_lshift(bn, bn, 32);
  BN_add_word(bn, (u32)(word & 0xFFFFFFFF));
}

void
loki::BigNum::set_binary(const loki::u8* bytes, size_t len)
{
  BN_lebin2bn(bytes, (int)len, bn);
}

bool
loki::BigNum::set_hex_str(const char* str)
{
  return BN_hex2bn(&bn, str) > 0;
}

loki::BigNum
loki::BigNum::exp(const loki::BigNum& bn1) const
{
  BigNum ret;

  BN_CTX* ctx = BN_CTX_new();
  BN_exp(ret.bn, bn, bn1.bn, ctx);
  BN_CTX_free(ctx);

  return ret;
}

loki::BigNum
loki::BigNum::mod_exp(const loki::BigNum& bn1, const loki::BigNum& bn2) const
{
  BN_CTX* ctx = BN_CTX_new();

  if (!ctx) {
    throw std::runtime_error("Failed to create BN_CTX");
  }

  BigNum ret;
  if (!BN_mod_exp(ret.bn, bn, bn1.bn, bn2.bn, ctx)) {
    BN_CTX_free(ctx);
    throw std::runtime_error("Failed to perform modular exponentiation");
  }

  BN_CTX_free(ctx);
  return ret;
}

std::vector<loki::u8>
loki::BigNum::to_byte_vector() const
{
  std::vector<loki::u8> buf(BN_num_bytes(bn));
  get_bytes(buf.data(), buf.size());
  return buf;
}

void
loki::BigNum::get_bytes(loki::u8* buf, size_t buf_size) const
{
  if (!BN_bn2lebinpad(bn, buf, (int)buf_size)) {
    throw std::runtime_error("Failed to convert BIGNUM to binary");
  }
}

void
loki::BigNum::set_rand(loki::i32 num_bits)
{
  BN_rand(bn, num_bits, 0, 1);
}

loki::BigNum&
loki::BigNum::operator+=(const loki::BigNum& other)
{
  BN_add(bn, bn, other.bn);
  return *this;
}

loki::BigNum
loki::BigNum::operator+(const loki::BigNum& other) const
{
  BigNum t(*this);
  return t += other;
}

loki::BigNum&
loki::BigNum::operator-=(const loki::BigNum& other)
{
  BN_sub(bn, bn, other.bn);
  return *this;
}

loki::BigNum
loki::BigNum::operator-(const loki::BigNum& other) const
{
  BigNum t(*this);
  return t -= other;
}

loki::BigNum&
loki::BigNum::operator*=(const loki::BigNum& other)
{
  BN_CTX* ctx = BN_CTX_new();
  BN_mul(bn, bn, other.bn, ctx);
  BN_CTX_free(ctx);

  return *this;
}

loki::BigNum
loki::BigNum::operator*(const loki::BigNum& other) const
{
  BigNum t(*this);
  return t *= other;
}

loki::BigNum&
loki::BigNum::operator/=(const loki::BigNum& other)
{
  BN_CTX* ctx = BN_CTX_new();
  BN_div(bn, nullptr, bn, other.bn, ctx);
  BN_CTX_free(ctx);

  return *this;
}

loki::BigNum
loki::BigNum::operator/(const loki::BigNum& other) const
{
  BigNum t(*this);
  return t /= other;
}

loki::BigNum&
loki::BigNum::operator%=(const loki::BigNum& other)
{
  BN_CTX* ctx = BN_CTX_new();
  BN_mod(bn, bn, other.bn, ctx);
  BN_CTX_free(ctx);

  return *this;
}

loki::BigNum
loki::BigNum::operator%(const loki::BigNum& other) const
{
  BigNum t(*this);
  return t %= other;
}

loki::BigNum&
loki::BigNum::operator<<=(int n)
{
  BN_lshift(bn, bn, n);
  return *this;
}

loki::BigNum
loki::BigNum::operator<<(int n) const
{
  BigNum t(*this);
  return t <<= n;
}

int
loki::BigNum::cmp(const loki::BigNum& other) const
{
  return BN_cmp(bn, other.bn);
}

std::string
loki::BigNum::as_hex_str() const
{
  char* ch = BN_bn2hex(bn);
  std::string ret = ch;
  OPENSSL_free(ch);
  return ret;
}

