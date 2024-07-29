#pragma once

#include <thread>

#include "engine/engine_app.h"
#include "engine/network/auth_connection.h"
#include "engine/render/shader.h"
#include "glm/detail/type_mat4x4.hpp"
#include "glm/vec3.hpp"

class GameApp : public loki::EngineApp
{
public:
  ~GameApp() override;

protected:
  void post_init() override;
  void update() override;
  void draw_ui() override;
  void draw() override;

private:
  glm::vec3 background{ 0.144f, 0.186f, 0.311f };
  std::uint32_t vao{};
  std::uint32_t vbo{};
  std::uint32_t texture{};
  loki::ProgramHandle program{};
  glm::mat4 model{ 1.f };
  glm::mat4 view{};
  glm::mat4 projection{};
  std::unique_ptr<loki::AuthConnection> auth_conn;
};

