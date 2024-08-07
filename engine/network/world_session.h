#pragma once

#include <shared_mutex>
#include <string_view>
#include <thread>

#include "auth_crypt.h"
#include "auth_session.h"
#include "engine/utils/byte_buffer.h"
#include "engine/utils/types.h"
#include "opcodes.h"
#include "sockpp/tcp_connector.h"

namespace loki {

  class WorldSession
  {
  public:
    struct ServerPacketHeader
    {
      u32 size{};
      std::array<u8, 5> header{};
    };

    struct ClientPacketHeader
    {
      u16 size{};
      u32 command{};
    };

  public:
    explicit WorldSession(const std::weak_ptr<AuthSession>& auth_session, u8 realm_id, std::string_view host, u16 port);
    ~WorldSession();

  private:
    void handle_connection();
    void read_next_packet();
    void process_command(u16 command);
    void handle_auth_challenge();

  private:
    u8 realm_id;
    std::weak_ptr<AuthSession> auth_session;
    sockpp::tcp_connector connector;
    sockpp::tcp_socket socket;
    std::thread thread;
    AuthCrypt auth_crypt;
    std::atomic_bool running;
    ByteBuffer buffer;
    std::queue<ByteBuffer> outgoing_messages;
    bool encrypted = false;
  };

} // namespace loki
