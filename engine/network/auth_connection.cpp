#include "auth_connection.h"

namespace config {

  constexpr const char game[] = "WoW";
  constexpr int build = 12'340;
  constexpr const char locale[] = "enUS";
  constexpr const char os[] = "Win";
  constexpr const char platform[] = "x86";
  constexpr int major_version = 3;
  constexpr int minor_version = 3;
  constexpr int patch_version = 5;
  constexpr int timezone = 0;

} // namespace config

struct PacketAuth : public loki::Packet
{
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
};

struct PaketAuthChallengeRequest : PacketAuth
{
  LOKI_DECLARE_PACKET_FIELD(protocol_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(packet_size, loki::i16);
  LOKI_DECLARE_PACKET_ARRAY(game_name, loki::u8, 4);
  LOKI_DECLARE_PACKET_FIELD(major_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(minor_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(patch_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(build, loki::i16);
  LOKI_DECLARE_PACKET_ARRAY(platform, loki::u8, 4);
  LOKI_DECLARE_PACKET_ARRAY(os, loki::u8, 4);
  LOKI_DECLARE_PACKET_ARRAY(country, loki::u8, 4);
  LOKI_DECLARE_PACKET_FIELD(timezone, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(ip_address, loki::u32);
  LOKI_DECLARE_PACKET_BLOCK(login);
};

struct PaketAuthChallengeResponse : PacketAuth
{
  LOKI_DECLARE_PACKET_FIELD(protocol_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(status, loki::i8);
  LOKI_DECLARE_PACKET_ARRAY(B, loki::u8, 32);
  LOKI_DECLARE_PACKET_BLOCK(g);
  LOKI_DECLARE_PACKET_BLOCK(N);
  LOKI_DECLARE_PACKET_ARRAY(s, loki::u8, 32);
  LOKI_DECLARE_PACKET_ARRAY(crc_salt, loki::u8, 16);
  LOKI_DECLARE_PACKET_FIELD(two_factor_enabled, loki::i8);
};

struct PaketAuthLogonProofRequest : PacketAuth
{
  LOKI_DECLARE_PACKET_ARRAY(A, loki::u8, 32);
  LOKI_DECLARE_PACKET_ARRAY(client_M, loki::u8, 20);
  LOKI_DECLARE_PACKET_ARRAY(crc_hash, loki::u8, 20);
  LOKI_DECLARE_PACKET_FIELD(number_of_keys, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(two_factor_enabled, loki::i8);
};

struct PaketAuthLogonProofResponse : PacketAuth
{
  LOKI_DECLARE_PACKET_FIELD(status, loki::i8);
  LOKI_DECLARE_PACKET_ARRAY(server_M, loki::u8, 20);
  LOKI_DECLARE_PACKET_FIELD(acount_flags, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(hardware_survey_id, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(unknown_flags, loki::u16);
};

struct PacketAuthRealmListRequest : PacketAuth
{
  LOKI_DECLARE_PACKET_FIELD(unknown, loki::u32);
};

struct PacketAuthRealmListHead : PacketAuth
{
  LOKI_DECLARE_PACKET_FIELD(packet_size, loki::u16);
  LOKI_DECLARE_PACKET_FIELD(uknown, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(number_of_realms, loki::u16);
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

loki::AuthConnection::AuthConnection(std::string_view host, loki::u16 port)
  : connector({ std::string(host), port })
  , thread()
  , running(true)
  , state(AuthConnectionState::INVALID)
{
  if (connector) {
    thread = std::thread(
        [this](sockpp::tcp_socket sock) {
          handle_connection(std::move(sock));
        },
        connector.clone());
  }
}

loki::AuthConnection::~AuthConnection()
{
  running = false;
  thread.join();
}

void
loki::AuthConnection::handle_connection(sockpp::tcp_socket sock)
{
  using namespace std::chrono_literals;

  socket = std::move(sock);

  while (running) {
    switch (state) {
      case AuthConnectionState::CHALLENGE:
        handle_challenge();
        break;
      case AuthConnectionState::LOGON_PROOF:
        handle_logon_proof();
        break;
      case AuthConnectionState::REALM_LIST:
        handle_realm_list();
        break;
      default:
        break;
    }

    if (state == AuthConnectionState::REALM_LIST) {
      std::this_thread::sleep_for(1s);
    }
  }

  socket.shutdown();
}

void
loki::AuthConnection::login(std::string_view username, std::string_view password)
{
  auto to_uppercase = [](std::string& string) {
    std::transform(string.begin(), string.end(), string.begin(), ::toupper);
  };

  username_uppercase = username;
  to_uppercase(username_uppercase);

  password_uppercase = password;
  to_uppercase(password_uppercase);

  state = AuthConnectionState::CHALLENGE;
}

void
loki::AuthConnection::handle_challenge()
{
  {
    PaketAuthChallengeRequest pkt;
    pkt.command.set(0);
    pkt.protocol_version.set(8);
    pkt.packet_size.set(static_cast<loki::i16>(30 + username_uppercase.length()));
    pkt.game_name.set(config::game);
    pkt.major_version.set(config::major_version);
    pkt.minor_version.set(config::minor_version);
    pkt.patch_version.set(config::patch_version);
    pkt.build.set(config::build);
    pkt.platform.set(config::platform);
    pkt.os.set(config::os);
    pkt.country.set(config::locale);
    pkt.timezone.set(config::timezone);
    pkt.ip_address.set(0);
    pkt.login.set(username_uppercase);

    spdlog::info("[Auth Challenge Request]");
    pkt.for_each_field([](const loki::PacketField& field) {
      spdlog::info("{}: {}", field.get_name(), field.to_string());
    });

    pkt.save_buffer(buffer);
    buffer.send(socket);
  }

  {
    PaketAuthChallengeResponse pkt;
    buffer.recv(socket);
    pkt.load_buffer(buffer);

    spdlog::info("[Auth Challenge Request]");
    pkt.for_each_field([](const loki::PacketField& field) {
      spdlog::info("{}: {}", field.get_name(), field.to_string());
    });

    loki::BigNum N = loki::BigNum::from_binary(*pkt.N);
    loki::BigNum g = loki::BigNum::from_binary(*pkt.g);
    srp6 = loki::SRP6(N, g);
    srp6->generate(*pkt.s, *pkt.B, username_uppercase, password_uppercase);
  }

  state = AuthConnectionState::LOGON_PROOF;
}

void
loki::AuthConnection::handle_logon_proof()
{
  {
    PaketAuthLogonProofRequest pkt;
    pkt.command.set(1);
    pkt.A.set(srp6->get_A());
    pkt.client_M.set(srp6->get_client_M());
    pkt.crc_hash.set(srp6->get_crc_hash());
    pkt.number_of_keys.set(0);
    pkt.two_factor_enabled.set(0);

    spdlog::info("[Auth Logon Proof Request]");
    pkt.for_each_field([](const loki::PacketField& field) {
      spdlog::info("{}: {}", field.get_name(), field.to_string());
    });

    pkt.save_buffer(buffer);
    buffer.send(socket);
  }

  {
    PaketAuthLogonProofResponse pkt;
    buffer.recv(socket);
    pkt.load_buffer(buffer);

    spdlog::info("[Auth Logon Proof Response]");
    pkt.for_each_field([](const loki::PacketField& field) {
      spdlog::info("{}: {}", field.get_name(), field.to_string());
    });
  }

  state = AuthConnectionState::REALM_LIST;
}

void
loki::AuthConnection::handle_realm_list()
{
  spdlog::info("Checking ream list...");

  PacketAuthRealmListRequest pkt;
  pkt.command.set(0x10); // Command: Realm List (0x10)
  pkt.unknown.set(0);

  spdlog::info("[Realm List]");
  pkt.for_each_field([](const loki::PacketField& field) {
    spdlog::info("{}: {}", field.get_name(), field.to_string());
  });

  pkt.save_buffer(buffer);
  buffer.send(socket);

  PacketAuthRealmListHead pkt_head;
  buffer.recv(socket);
  pkt_head.load_buffer(buffer);

  {
    std::unique_lock lock(realms_mutex);
    realms.clear();
  }

  for (int i = 0; i < *pkt_head.number_of_realms; ++i) {
    PacketAuthRealmListBody pkt_body;
    pkt_body.load_buffer(buffer);
    {
      std::unique_lock lock(realms_mutex);
      Realm& realm = realms.emplace_back();
      realm.name = *pkt_body.name;
      realm.server_socket = *pkt_body.server_socket;
    }
  }
}

auto
loki::AuthConnection::get_realms() const -> std::vector<Realm>
{
  std::shared_lock lock(realms_mutex);
  return realms;
}
