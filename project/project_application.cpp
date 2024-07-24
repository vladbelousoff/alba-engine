#include "engine/network/packet.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"

#include "project_application.h"

#include "engine/datasource/mpq/mpq_chain.h"
#include "engine/network/byte_buffer.h"
#include "engine/utils/endianness.h"
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

void ProjectApplication::post_init()
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

void ProjectApplication::update()
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

class AuthChallenge : public loki::Packet
{
public:
  LOKI_DECLARE_PACKET_FIELD(command, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(protocol_version, loki::i8);
  LOKI_DECLARE_PACKET_FIELD(packet_size, loki::i16);
  LOKI_DECLARE_PACKET_FIELD(game_name, loki::u8, 4);
};

void ProjectApplication::draw_ui()
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

        std::string username_str = std::string(username);
        to_uppercase(username_str);

        std::string password_str = std::string(password);
        to_uppercase(password_str);

        loki::ByteBuffer challenge(loki::Endianness::LittleEndian);

        challenge.append<int8_t>(0); // auth challenge
        challenge.append<int8_t>(8); // protocol version
        challenge.append<int16_t>(30 + username_str.length());
        challenge.append(config::game);
        challenge.append<int8_t>(0); // null terminator
        challenge.append<int8_t>(config::major_version);
        challenge.append<int8_t>(config::minor_version);
        challenge.append<int8_t>(config::patch_version);
        challenge.append<int16_t>(config::build);
        challenge.append(config::platform);
        challenge.append<int8_t>(0); // null terminator
        challenge.append(config::os);
        challenge.append<int8_t>(0); // null terminator
        challenge.append(config::locale);
        challenge.append<uint32_t>(config::timezone);
        challenge.append<uint32_t>(0x0100007f); // ip 127.0.0.1
        challenge.append<uint8_t>(username_str.length());
        challenge.append(username_str, false);

        challenge.send(conn);
        challenge.receive(conn);

        auto command = challenge.read<uint8_t>();
        spdlog::info("Protocol: {}", command);

        auto protocol = challenge.read<uint8_t>();
        spdlog::info("Protocol: {}", protocol);

        auto status = challenge.read<uint8_t>();
        spdlog::info("Status: {}", status);

        std::array<char, 32> b{};
        challenge.read(b);

        auto g_len = challenge.read<uint8_t>();
        spdlog::info("SRP G Length: {}", g_len);
        std::vector<char> g(g_len);
        challenge.read_bytes(g.data(), g_len);

        auto n_len = challenge.read<uint8_t>();
        spdlog::info("SRP N Length: {}", n_len);
        std::vector<char> n(n_len);
        challenge.read_bytes(n.data(), n_len);

        std::array<char, 32> s{};
        challenge.read(s);

        std::array<char, 16> crc_salt{};
        challenge.read(crc_salt);

        auto two_factor_enabled = challenge.read<uint8_t>();
        spdlog::info("Two Factor Enabled: {}", two_factor_enabled);

      } else {
        spdlog::error("Error connecting to server at {}", sockpp::inet_address(host, port).to_string());
        spdlog::error("{}", conn.last_error_str());
      }
    }
  }
  ImGui::End();
}

void ProjectApplication::draw()
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
