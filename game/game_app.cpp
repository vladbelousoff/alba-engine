#include "engine/network/auth_connection.h"

#include "game_app.h"

#include "engine/utils/types.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "spdlog/spdlog.h"

struct
{
  float distance_to_origin{ 6.7f };
  float phi{ 5.182f }, theta{ 5.716f };
} camera;

GameApp::~GameApp()
{
  auth_conn.reset();
}

void
GameApp::post_init()
{
  sockpp::initialize();

  // loki::MPQChain chain{ get_root_path() / "Data" };
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

void
GameApp::draw_ui()
{
  if (ImGui::Begin("Auth")) {
    static char host[32] = "localhost";
    ImGui::InputText("Host", host, sizeof(host));
    static int port = 3'724;
    ImGui::InputInt("Port", &port);
    static char username[32] = "test";
    ImGui::InputText("Username", username, sizeof(username));
    static char password[32] = "test";
    ImGui::InputText("Password", password, sizeof(password));

    if (ImGui::Button("Connect")) {
      spdlog::info("Connecting to auth-server @{}:{}...", host, port);

      auth_conn = std::make_unique<loki::AuthConnection>(host, (loki::u16)port);
      auth_conn->login(username, password);
    }
  }
  ImGui::End();
}

void
GameApp::draw()
{
  glClearColor(background.r, background.g, background.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
