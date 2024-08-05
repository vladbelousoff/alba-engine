#pragma once

#include <thread>

#include "engine/engine_app.h"
#include "engine/network/auth_session.h"
#include "engine/network/world_session.h"
#include "engine/render/shader.h"
#include "glm/detail/type_mat4x4.hpp"
#include "glm/vec3.hpp"

class GameApp : public loki::EngineApp
{
protected:
  bool on_init() override;
  void on_term() override;

  void on_update() override;
  void on_render() override;
  void on_gui() override;

private:
  glm::vec3 background{ 0.144f, 0.186f, 0.311f };
  glm::mat4 model{ 1.f };
  glm::mat4 view{};
  glm::mat4 projection{};
  std::shared_ptr<loki::AuthSession> auth_session;
  std::shared_ptr<loki::WorldSession> world_session;
};

