#pragma once

#include <memory>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "engine/utils/types.h"
#include "packet.h"
#include "sockpp/tcp_connector.h"
#include "srp6.h"

namespace loki {

  struct PacketAuthRealmListBody
  {
    loki::u8 type{};
    loki::u8 locked{};
    loki::u8 flags{};
    std::string name;
    std::string server_socket;
    loki::u8 population_level{};
    loki::u8 number_of_characters{};
    loki::u8 category{};
    loki::u8 realm_id{};
  };

  enum class AuthSessionState : i8
  {
    INVALID = -1,
    CHALLENGE = 0,
    LOGON_PROOF = 1,
    REALM_LIST = 2,
  };

  class AuthSession
  {
  public:
    explicit AuthSession(std::string_view host, u16 port);
    ~AuthSession();

  public:
    void login(std::string_view username, std::string_view password);
    auto get_realms() const -> std::vector<PacketAuthRealmListBody>;

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
    std::atomic<AuthSessionState> state;
    std::optional<loki::SRP6> srp6;
    ByteBuffer buffer;
    mutable std::shared_mutex realms_mutex;
    std::vector<PacketAuthRealmListBody> realms;
  };

} // namespace loki
