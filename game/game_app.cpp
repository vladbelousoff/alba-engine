#include "engine/network/srp6.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"

#include "game_app.h"

#include "engine/network/byte_buffer.h"
#include "engine/network/packet.h"
#include "engine/utils/types.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "spdlog/spdlog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#define DRAW_CUBE 0

#if DRAW_CUBE
#include "engine/render/shader.h"
#endif

using namespace std;
using namespace std::chrono;

#if 0
namespace game::config {
  static const char* name = "Wow ";
  static const int build = 12'340;
  static const char* version = "3.3.5";
  static const int timezone = 0;
  static const char* locale = "enUS";
  static const char* os = "Win";
  static const char* platform = "x86";
}
#endif

static std::string default_shader_vert =
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_position;\n"
    "layout (location = 1) in vec2 a_texcoord;\n"
    "uniform mat4 u_model;\n"
    "uniform mat4 u_view;\n"
    "uniform mat4 u_projection;\n"
    "out vec3 color;\n"
    "out vec2 texcoord;\n"
    "void main() {\n"
    "  gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);\n"
    "  texcoord = a_texcoord;\n"
    "}\n";

static std::string default_shader_frag =
    "#version 330 core\n"
    "out vec4 frag_color;\n"
    "in vec2 texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "  frag_color = texture(u_texture, texcoord);\n"
    "}\n";

#if DRAW_CUBE
static float vertices[] = { -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f,
  1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,
  0.0f, 0.0f,

  -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f,
  0.5f, 1.0f, 0.0f,

  0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f,
  1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f,
  -0.5f, 0.0f, 1.0f,

  -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f,
  0.0f, 1.0f };
#endif

struct
{
  float distance_to_origin{ 6.7f };
  float phi{ 5.182f }, theta{ 5.716f };
} camera;

void
GameApp::post_init()
{
  sockpp::initialize();

  // loki::MPQChain chain{ get_root_path() / "Data" };

#if 0
  HANDLE mpq_handle;
  HANDLE file_handle;

  const auto mpq_path = absolute(get_root_path() / "Data" / "common.MPQ");
  if (!SFileOpenArchive(mpq_path.string().c_str(), 0, 0, &mpq_handle)) {
    spdlog::error("Error opening MPQ archive: {}", mpq_path.string());
    return;
  }

  std::string file_name = "file/to/read.txt";
  if (!SFileOpenFileEx(mpq_handle, file_name.c_str(), SFILE_OPEN_FROM_MPQ, &file_handle)) {
    spdlog::error("Error opening file: {}", file_name);
    SFileCloseArchive(mpq_handle);
    return;
  }
#endif

#if DRAW_CUBE
  // Gen VAO & VBO
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  // Bind VAO
  glBindVertexArray(vao);

  // Bind VBO and copy vertices data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
  glEnableVertexAttribArray(0);

  // Texture attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Generate a texture
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load and generate the texture
  std::filesystem::path texture_path = absolute(get_root_path() / std::filesystem::path{ "data/container2.png" });

  glm::ivec2 texture_size;
  int nr_channels;

  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(texture_path.string().c_str(), &texture_size.x, &texture_size.y, &nr_channels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  } else {
    spdlog::error("Failed to load texture {}!", texture_path.string());
  }

  glEnable(GL_DEPTH_TEST);

  loki::ShaderHandle frag = loki::ShaderManager::create_shader(default_shader_frag, loki::ShaderType::FRAG);
  loki::ShaderHandle vert = loki::ShaderManager::create_shader(default_shader_vert, loki::ShaderType::VERT);
  program = loki::ShaderManager::create_program(vert, frag);

  loki::ShaderManager::use_program(program, [&](const loki::UniformManager& manager) {
    manager.set_uniform(loki::StringID{ "u_texture" }, 0);
  });
#endif
}

void
GameApp::update()
{
  float x = camera.distance_to_origin * glm::sin(camera.phi) * glm::cos(camera.theta);
  float y = camera.distance_to_origin * glm::cos(camera.phi);
  float z = camera.distance_to_origin * glm::sin(camera.phi) * glm::sin(camera.theta);

  view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

  glm::ivec2 window_size;
  glfwGetWindowSize(get_window(), &window_size.x, &window_size.y);
  glViewport(0, 0, window_size.x, window_size.y);

  projection = glm::perspective(glm::radians(45.0f), (float)window_size.x / (float)window_size.y, 0.1f, 100.0f);
}

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

struct PaketAuthChallengeRequest : public loki::Packet
{
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
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

struct PaketAuthChallengeResponse : public loki::Packet
{
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(protocol_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(status, loki::i8);
  LOKI_DECLARE_PACKET_ARRAY(B, loki::u8, 32);
  LOKI_DECLARE_PACKET_BLOCK(g);
  LOKI_DECLARE_PACKET_BLOCK(N);
  LOKI_DECLARE_PACKET_ARRAY(s, loki::u8, 32);
  LOKI_DECLARE_PACKET_ARRAY(crc_salt, loki::u8, 16);
  LOKI_DECLARE_PACKET_FIELD(two_factor_enabled, loki::i8);
};

struct PaketLogonProofRequest : public loki::Packet
{
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
  LOKI_DECLARE_PACKET_ARRAY(A, loki::u8, 32);
  LOKI_DECLARE_PACKET_ARRAY(client_M, loki::u8, 20);
  LOKI_DECLARE_PACKET_ARRAY(crc_hash, loki::u8, 20);
  LOKI_DECLARE_PACKET_FIELD(number_of_keys, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(two_factor_enabled, loki::i8);
};

struct PaketLogonProofResponse : public loki::Packet
{
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(status, loki::i8);
  LOKI_DECLARE_PACKET_ARRAY(server_M, loki::u8, 20);
  LOKI_DECLARE_PACKET_FIELD(acount_flags, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(hardware_survey_id, loki::u32);
  LOKI_DECLARE_PACKET_FIELD(unknown_flags, loki::u16);
};

void
GameApp::draw_ui()
{
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Create")) {
      }
      if (ImGui::MenuItem("Open", "Ctrl+O")) {
      }
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
      }
      if (ImGui::MenuItem("Save as..")) {
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (ImGui::Begin("Settings")) {
    ImGui::Text("Frame: %.3fms", get_delta_time() * 1000.f);
    ImGui::Text("FPS: %.0f", get_fps());
    ImGui::Text("Background Color");
    ImGui::SliderFloat("Red", &background.r, 0.0f, 1.0f);
    ImGui::SliderFloat("Green", &background.g, 0.0f, 1.0f);
    ImGui::SliderFloat("Blue", &background.b, 0.0f, 1.0f);
    ImGui::Text("Orbit Camera");
    ImGui::SliderFloat("Phi", &camera.phi, 0.0f, glm::pi<float>());
    ImGui::SliderFloat("Theta", &camera.theta, 0.0f, glm::pi<float>() * 2.f);
    ImGui::SliderFloat("Distance", &camera.distance_to_origin, 0.0f, 100.0f);
  }
  ImGui::End();

  if (ImGui::Begin("Auth")) {
    static char host[32] = "localhost";
    ImGui::InputText("Host", host, sizeof(host));
    static int port = 3'724;
    ImGui::InputInt("Port", &port);
    static char username[32] = "test";
    ImGui::InputText("Username", username, sizeof(username));
    static char password[32] = "test";
    ImGui::InputText("Password", password, sizeof(password));

    auto to_uppercase = [](std::string& string) {
      std::transform(string.begin(), string.end(), string.begin(), ::toupper);
    };

    if (ImGui::Button("Connect")) {
      spdlog::info("Connecting to auth-server @{}:{}...", host, port);

      sockpp::tcp_connector conn({ host, (std::uint16_t)port });
      if (conn) {
        spdlog::info("Created a connection from {}", conn.address().to_string());
        spdlog::info("Created a connection to {}", conn.peer_address().to_string());

        std::string username_uppercase = std::string(username);
        to_uppercase(username_uppercase);

        std::string password_uppercase = std::string(password);
        to_uppercase(password_uppercase);

        loki::ByteBuffer byte_buffer{};

        PaketAuthChallengeRequest auth_request;
        auth_request.command.set(0);
        auth_request.protocol_version.set(8);
        auth_request.packet_size.set(static_cast<loki::i16>(30 + username_uppercase.length()));
        auth_request.game_name.set(config::game);
        auth_request.major_version.set(config::major_version);
        auth_request.minor_version.set(config::minor_version);
        auth_request.patch_version.set(config::patch_version);
        auth_request.build.set(config::build);
        auth_request.platform.set(config::platform);
        auth_request.os.set(config::os);
        auth_request.country.set(config::locale);
        auth_request.timezone.set(config::timezone);
        auth_request.ip_address.set(0);
        auth_request.login.set(username_uppercase);

        spdlog::info("---- Auth Request ----");
        auth_request.for_each_field([](const loki::PacketField& field) {
          spdlog::info("{}: {}", field.get_name(), field.to_string());
        });

        auth_request.save_buffer(byte_buffer);
        byte_buffer.send(conn);

        PaketAuthChallengeResponse auth_response;
        byte_buffer.receive(conn);
        auth_response.load_buffer(byte_buffer);

        spdlog::info("---- Auth Response ----");
        auth_response.for_each_field([](const loki::PacketField& field) {
          spdlog::info("{}: {}", field.get_name(), field.to_string());
        });

        loki::BigNum N = loki::BigNum::from_binary(*auth_response.N);
        loki::BigNum g = loki::BigNum::from_binary(*auth_response.g);
        loki::SRP6 srp_client(N, g);

        auto [session_key, client_M, crc_hash] = srp_client.generate(*auth_response.s, *auth_response.B, username_uppercase, password_uppercase);

        PaketLogonProofRequest logon_proof_request;
        logon_proof_request.command.set(1);
        logon_proof_request.A.set(srp_client.get_A());
        logon_proof_request.client_M.set(client_M);
        logon_proof_request.crc_hash.set(crc_hash);
        logon_proof_request.number_of_keys.set(0);
        logon_proof_request.two_factor_enabled.set(0);

        spdlog::info("---- Logon Proof Request ----");
        logon_proof_request.for_each_field([](const loki::PacketField& field) {
          spdlog::info("{}: {}", field.get_name(), field.to_string());
        });

        byte_buffer.reset();
        logon_proof_request.save_buffer(byte_buffer);
        byte_buffer.send(conn);

        byte_buffer.reset();
        byte_buffer.receive(conn);
        PaketLogonProofResponse logon_proof_response;
        logon_proof_response.load_buffer(byte_buffer);

        spdlog::info("---- Logon Proof Response ----");
        logon_proof_response.for_each_field([](const loki::PacketField& field) {
          spdlog::info("{}: {}", field.get_name(), field.to_string());
        });

        auto server_M = loki::SHA1::get_digest_of(srp_client.get_A(), client_M, session_key);
        DEBUG_ASSERT(*logon_proof_response.server_M == server_M);

      } else {
        spdlog::error("Error connecting to server at {}", sockpp::inet_address(host, port).to_string());
        spdlog::error("{}", conn.last_error_str());
      }
    }
  }
  ImGui::End();
}

void
GameApp::draw()
{
  glClearColor(background.r, background.g, background.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if DRAW_CUBE
  loki::ShaderManager::use_program(program, [&](const loki::UniformManager& manager) {
    manager.set_uniform(loki::StringID{ "u_view" }, view);
    manager.set_uniform(loki::StringID{ "u_model" }, model);
    manager.set_uniform(loki::StringID{ "u_projection" }, projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
  });
#endif
}
