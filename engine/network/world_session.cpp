#include "world_session.h"

#include "engine/config.h"
#include "engine/crypto/crypto_hash.h"
#include "engine/crypto/crypto_random.h"
#include "opcodes.h"

loki::WorldSession::WorldSession(const std::shared_ptr<AuthSession>& auth_session, std::string_view host, loki::u16 port)
  : auth_session(auth_session)
  , connector({ std::string(host), port })
  , thread()
  , running(true)
{
  if (connector) {
    thread = std::thread(
        [this](sockpp::tcp_socket sock) {
          socket = std::move(sock);
          handle_connection();
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
loki::WorldSession::handle_connection()
{
  spdlog::info("Connected to {}", socket.peer_address().to_string());

  ssize_t n = buffer.recv(socket);
  if (n <= 0) {
    return;
  }

  buffer.read<loki::u16>(); // header
  auto command = buffer.read<loki::u16>();

  auto one = buffer.read<u32>();
  DEBUG_ASSERT(one == 0x1);

  std::array<loki::u8, 4> auth_seed{};
  buffer.read(auth_seed);

  if (command == SMSG_AUTH_CHALLENGE) {
    spdlog::info("Received MSG Auth Challenge");
    buffer.read<loki::u8>(); // 0x1

    std::array<loki::u8, 4> t{};

    auto local_challenge = crypto::get_random_bytes<4>();
    auto session_key = auth_session.lock()->get_session_key();
    auto& username = auth_session.lock()->get_username();

    using namespace crypto;

    buffer.reset();

    struct
    {
      u32 build = config::build;
      u32 login_server_id = 0;
      std::string account{};
      u32 local_server_type = 0;
      std::array<u8, 4> local_challenge{};
      u32 region_id = 0;
      u32 battle_group_id = 0;
      u32 realm_id = 1;
      u64 dos_response = 0;
      SHA1::Digest digest{};
      std::vector<u8> addon_info{};
    } auth_info;

    auth_info.local_challenge = local_challenge;
    auth_info.digest = SHA1::get_digest_of(username, t, local_challenge, auth_seed, *session_key);
    auth_info.account = username;

    ClientPacketHeader client_header{};
    client_header.size = htons(username.length() + auth_info.addon_info.size() + 62);
    client_header.command = CMSG_AUTH_SESSION;

    buffer.save_buffer(client_header);
    buffer.save_buffer(auth_info);
    buffer.send(socket);
  }

  buffer.reset();
  ssize_t n1 = buffer.recv(socket);
  spdlog::info("n1 = {}", n1);

  socket.shutdown();
}
