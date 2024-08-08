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
  , auth_crypt()
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

  const auto& session_key = auth_session.lock()->get_session_key();
  auth_crypt.init(*session_key);

  while (running) {
    buffer.reset();
    read_incoming_packets();
  }

  socket.shutdown();
}

void
loki::WorldSession::read_incoming_packets()
{
  ssize_t packet_size = buffer.recv(socket);
  if (packet_size <= 0) {
    return;
  }

  if (encrypted) {
    while (buffer.can_read()) {
      read_next_packet();
    }
  } else {
    buffer.read<u16>(); // header, unused data (probably size?)
    u16 command = buffer.read<u16>();
    process_command(command);
  }
}

void
loki::WorldSession::read_next_packet()
{
  auto cur_pos = buffer.get_r_pos();
  std::array<u8, 4> header{};

  buffer.load_buffer(header);
  auth_crypt.decrypt_recv(header.data(), 4 /* small size packet, there's also 5-size packets, but it's not our case so far */);

  auto size = htons(*reinterpret_cast<u16*>(&header[0]));
  u16 command = *reinterpret_cast<u16*>(&header[2]);
  spdlog::info("Packet size: {}", size);

  process_command(command);
  buffer.set_r_pos(cur_pos + size + 2);
}

void
loki::WorldSession::process_command(loki::u16 command)
{
  switch (command) {
    case SMSG_AUTH_CHALLENGE:
      handle_auth_challenge();
      break;
    case SMSG_AUTH_RESPONSE:
      handle_auth_response();
      break;
    default:
      spdlog::info("Unkown command: {:#x}", command);
      break;
  }
}

void
loki::WorldSession::handle_auth_challenge()
{
  spdlog::info("Receiving SMSG_AUTH_CHALLENGE");

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

  buffer.reset();
  buffer.save_buffer(client_header);
  buffer.save_buffer(auth_info);
  buffer.send(socket);

  spdlog::info("Sending CMSG_AUTH_SESSION");

  encrypted = true;
}

void
loki::WorldSession::handle_auth_response()
{
  spdlog::info("Receiving SMSG_AUTH_RESPONSE");

  u8 status = buffer.read<u8>();
  ASSERT(status == 12, "You're not the first in the queue"); // AUTH_OK code
}
