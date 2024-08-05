#include "world_session.h"

#include "engine/config.h"
#include "engine/crypto/crypto_hash.h"
#include "engine/crypto/crypto_random.h"
#include "opcodes.h"

loki::WorldSession::WorldSession(const std::weak_ptr<AuthSession>& auth_session, u8 realm_id, std::string_view host, loki::u16 port)
  : auth_session(auth_session)
  , realm_id(realm_id)
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

  while (running) {
    buffer.reset();
    read_next_packet();
    send_next_packet();
  }

  socket.shutdown();
}

void
loki::WorldSession::read_next_packet()
{
  ssize_t n = buffer.recv(socket);
  if (n <= 0) {
    return;
  }

  buffer.read<loki::u16>(); // header
  u16 command = buffer.read<loki::u16>();
  process_command(command);
}

void
loki::WorldSession::send_next_packet()
{
  if (!outgoing_messages.empty()) {
    outgoing_messages.back().send(socket);
    outgoing_messages.pop();
  }
}

void
loki::WorldSession::process_command(loki::u16 command)
{
  switch (command) {
    case SMSG_AUTH_CHALLENGE:
      handle_auth_challenge();
      break;
    default:
      spdlog::info("Unkown command: {}", command);
      break;
  }
}

void
loki::WorldSession::handle_auth_challenge()
{
  auto one = buffer.read<u32>();
  DEBUG_ASSERT(one == 0x1);

  std::array<loki::u8, 4> auth_seed{};
  buffer.read(auth_seed);

  std::array<loki::u8, 4> t{};

  auto local_challenge = crypto::get_random_bytes<4>();
  auto session_key = auth_session.lock()->get_session_key();
  auto& username = auth_session.lock()->get_username();

  struct
  {
    u32 build = config::build;
    u32 login_server_id = 0;
    std::string account{};
    u32 local_server_type = 0;
    std::array<u8, 4> local_challenge{};
    u32 region_id = 0;
    u32 battle_group_id = 0;
    u32 realm_id = 0;
    u64 dos_response = 0;
    SHA1::Digest digest{};
    std::vector<u8> addon_info{};
  } auth_info;

  auth_info.realm_id = realm_id;
  auth_info.local_challenge = local_challenge;
  auth_info.digest = SHA1::get_digest_of(username, t, local_challenge, auth_seed, *session_key);
  auth_info.account = username;

  ClientPacketHeader client_header{};
  client_header.size = htons(username.length() + auth_info.addon_info.size() + 62);
  client_header.command = CMSG_AUTH_SESSION;

  ByteBuffer outgoing_buffer;
  outgoing_buffer.save_buffer(client_header);
  outgoing_buffer.save_buffer(auth_info);

  outgoing_messages.push(outgoing_buffer);

  spdlog::info("CMSG_AUTH_SESSION sent");
}
