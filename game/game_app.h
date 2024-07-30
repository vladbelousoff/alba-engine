#pragma once

#include <thread>

#include "engine/engine_app.h"
#include "engine/network/auth_session.h"
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
  glm::mat4 model{ 1.f };
  glm::mat4 view{};
  glm::mat4 projection{};
  std::unique_ptr<loki::AuthSession> auth_session;
};

