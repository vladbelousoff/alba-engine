#pragma once

#include <shared_mutex>
#include <string_view>
#include <thread>

#include "engine/utils/byte_buffer.h"
#include "engine/utils/types.h"
#include "sockpp/tcp_connector.h"

namespace loki {

  class WorldSession
  {
  public:
    explicit WorldSession(std::string_view host, u16 port);
    ~WorldSession();

  private:
    void handle_connection(sockpp::tcp_socket sock);

  private:
    sockpp::tcp_connector connector;
    sockpp::tcp_socket socket;
    std::thread thread;
    std::atomic_bool running;
    ByteBuffer buffer;
  };

} // namespace loki
