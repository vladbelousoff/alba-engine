#include "world_session.h"

loki::WorldSession::WorldSession(std::string_view host, loki::u16 port)
  : connector({ std::string(host), port })
  , thread()
  , running(true)
{
  if (connector) {
    thread = std::thread(
        [this](sockpp::tcp_socket sock) {
          handle_connection(std::move(sock));
        },
        connector.clone());
  }
}

loki::WorldSession::~WorldSession()
{
  running = false;
  thread.join();
}

void
loki::WorldSession::handle_connection(sockpp::tcp_socket sock)
{
  spdlog::info("Connected to {}", sock.peer_address().to_string());
}
