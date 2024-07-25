#include "srp.h"
#include "libassert/assert.hpp"

loki::SRP::SRP(const std::vector<loki::u8>& in_N, loki::u8 in_g)
{
  N = BN_bin2bn(in_N.data(), (int)in_N.size(), nullptr);

  g = BN_new();
  BN_set_word(g, in_g);

  A = BN_new();
  a = BN_new();
  u = BN_new();
  v = BN_new();
  S = BN_new();

  // Generate client secret
  BN_rand(a, 256, -1, 0);

  // Calculate A = g^a % N
  BN_CTX* ctx = BN_CTX_new();
  BN_mod_exp(A, g, a, N, ctx);
  BN_CTX_free(ctx);
}

loki::SRP::~SRP()
{
  BN_free(N);
  BN_free(g);
  BN_free(A);
  BN_free(a);
  BN_free(u);
  BN_free(v);
  BN_free(S);

  if (x) {
    BN_free(x);
  }

  if (B) {
    BN_free(B);
  }
}

void loki::SRP::hash_sha1(const std::vector<u8>& input, std::array<u8, 20>& output)
{
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
  EVP_DigestUpdate(ctx, input.data(), input.size());
  EVP_DigestFinal_ex(ctx, output.data(), nullptr);
  EVP_MD_CTX_free(ctx);
}

void loki::SRP::generate(const std::array<loki::u8, 32>& salt, const std::array<loki::u8, 32>& in_B, std::string_view username, std::string_view password)
{
  // Generate x = H(salt | H(username | ":" | password))
  std::vector<u8> username_password_hash_input;
  username_password_hash_input.insert(username_password_hash_input.end(), username.begin(), username.end());
  username_password_hash_input.push_back(':');
  username_password_hash_input.insert(username_password_hash_input.end(), password.begin(), password.end());

  std::array<u8, SHA_DIGEST_LENGTH> username_password_hash{};
  hash_sha1(username_password_hash_input, username_password_hash);

  std::vector<u8> x_input;
  x_input.insert(x_input.end(), salt.begin(), salt.end());
  x_input.insert(x_input.end(), username_password_hash.begin(), username_password_hash.end());

  std::array<u8, SHA_DIGEST_LENGTH> x_hash{};
  hash_sha1(x_input, x_hash);

  x = BN_bin2bn(x_hash.data(), SHA_DIGEST_LENGTH, nullptr);

  // Calculate v = g^x % N
  BN_CTX* ctx = BN_CTX_new();
  BN_mod_exp(v, g, x, N, ctx);

  // Convert B from binary to BIGNUM
  B = BN_bin2bn(in_B.data(), static_cast<int>(in_B.size()), nullptr);

  // Calculate u = H(A | B)
  int A_size = BN_num_bytes(A);
  int B_size = BN_num_bytes(B);

  DEBUG_ASSERT(A_size == A_bin.size());
  DEBUG_ASSERT(B_size == B_bin.size());

  BN_bn2bin(A, A_bin.data());
  BN_bn2bin(B, B_bin.data());

  std::vector<u8> u_input;
  u_input.insert(u_input.end(), A_bin.begin(), A_bin.end());
  u_input.insert(u_input.end(), B_bin.begin(), B_bin.end());

  std::array<u8, SHA_DIGEST_LENGTH> u_hash{};
  hash_sha1(u_input, u_hash);

  u = BN_bin2bn(u_hash.data(), SHA_DIGEST_LENGTH, nullptr);

  // Calculate S = (B - k * v) ^ (a + u * x) % N
  BIGNUM* k = BN_new();
  BN_set_word(k, 3); // k is typically set to a small constant, e.g., 3

  BIGNUM* kv = BN_new();
  BIGNUM* aux1 = BN_new();
  BIGNUM* aux2 = BN_new();

  BN_mod_mul(kv, k, v, N, ctx);
  BN_sub(aux1, B, kv);
  BN_mul(aux2, u, x, ctx);
  BN_add(aux2, a, aux2);
  BN_mod_exp(S, aux1, aux2, N, ctx);

  // Calculate M1 = H(A | B | S)
  std::vector<u8> S_bin(BN_num_bytes(S));
  BN_bn2bin(S, S_bin.data());

  std::vector<u8> M1_input;
  M1_input.insert(M1_input.end(), A_bin.begin(), A_bin.end());
  M1_input.insert(M1_input.end(), B_bin.begin(), B_bin.end());
  M1_input.insert(M1_input.end(), S_bin.begin(), S_bin.end());

  hash_sha1(M1_input, M1);

  // Clean up
  BN_free(k);
  BN_free(kv);
  BN_free(aux1);
  BN_free(aux2);
  BN_CTX_free(ctx);
}

const std::array<loki::u8, SHA_DIGEST_LENGTH>& loki::SRP::get_M1() const
{
  return M1;
}

const std::array<loki::u8, 32>& loki::SRP::get_A() const
{
  return A_bin;
}

