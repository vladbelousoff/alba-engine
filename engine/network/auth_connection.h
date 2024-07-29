#pragma once

#include <memory>
#include <queue>
#include <thread>

#include "engine/utils/types.h"
#include "packet.h"
#include "sockpp/tcp_connector.h"
#include "srp6.h"

namespace loki {

  enum class AuthConnectionState : i8
  {
    INVALID = -1,
    CHALLENGE = 0,
    LOGON_PROOF = 1,
    REALM_LIST = 2,
  };

  struct PacketAuthRealmListBody : loki::Packet
  {
    LOKI_DECLARE_PACKET_FIELD(type, loki::u8);
    LOKI_DECLARE_PACKET_FIELD(locked, loki::u8);
    LOKI_DECLARE_PACKET_FIELD(flags, loki::u8);
    LOKI_DECLARE_PACKET_STRING(name);
    LOKI_DECLARE_PACKET_STRING(server_socket);
    LOKI_DECLARE_PACKET_FIELD(population_level, loki::u32);
    LOKI_DECLARE_PACKET_FIELD(number_of_characters, loki::u8);
    LOKI_DECLARE_PACKET_FIELD(category, loki::u8);
    LOKI_DECLARE_PACKET_FIELD(realm_id, loki::u8);
  };

  class AuthConnection
  {
  public:
    explicit AuthConnection(std::string_view host, u16 port);
    ~AuthConnection();

  public:
    void login(std::string_view username, std::string_view password);

  private:
    void handle_connection(sockpp::tcp_socket sock);
    void handle_challenge();
    void handle_logon_proof();
    void handle_realm_list();

  private:
    std::string username_uppercase;
    std::string password_uppercase;
    sockpp::tcp_connector connector;
    sockpp::tcp_socket socket;
    std::thread thread;
    std::atomic_bool running;
    std::atomic<AuthConnectionState> state;
    std::optional<loki::SRP6> srp6;
    ByteBuffer buffer;
    std::vector<PacketAuthRealmListBody> realms;
  };

} // namespace loki
