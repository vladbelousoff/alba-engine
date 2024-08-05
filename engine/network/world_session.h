#pragma once

#include <shared_mutex>
#include <string_view>
#include <thread>

#include "auth_session.h"
#include "engine/utils/byte_buffer.h"
#include "engine/utils/types.h"
#include "opcodes.h"
#include "sockpp/tcp_connector.h"

namespace loki {

  struct ServerPacketHeader
  {
    u16 size;
    u32 command;

    auto is_valid_size() const -> bool
    {
      return size >= 4 && size <= 10'240;
    }

    auto is_valid_opcode() const -> bool
    {
      return command < NUM_MSG_TYPES;
    }
  };

  struct ClientPacketHeader
  {
    u16 size{};
    u32 command{};
  };

  class WorldSession
  {
  public:
    explicit WorldSession(const std::weak_ptr<AuthSession>& auth_session, u8 realm_id, std::string_view host, u16 port);
    ~WorldSession();

  private:
    void handle_connection();
    void read_next_packet();
    void send_next_packet();
    void process_command(u16 command);
    void handle_auth_challenge();

  private:
    u8 realm_id;
    std::weak_ptr<AuthSession> auth_session;
    sockpp::tcp_connector connector;
    sockpp::tcp_socket socket;
    std::thread thread;
    std::atomic_bool running;
    ByteBuffer buffer;
    std::queue<ByteBuffer> outgoing_messages;
  };

} // namespace loki
