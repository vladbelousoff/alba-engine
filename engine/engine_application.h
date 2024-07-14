#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace loki {

  struct EngineSettings
  {
    std::filesystem::path root_path;
  };

  class EngineApplication
  {
  public:
    virtual ~EngineApplication();

  public:
    auto launch(const std::shared_ptr<EngineSettings>& settings) -> int;

  protected:
    virtual void post_init() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void draw_ui() = 0;

  private:
    bool init();
    void term();

  protected:
    auto get_delta_time() const -> float
    {
      return delta_time;
    }

    auto get_fps() const -> float
    {
      return delta_time != 0.f ? 1.f / delta_time : 0.f;
    }

    auto get_root_path() const -> std::filesystem::path
    {
      return settings->root_path;
    }

    auto get_window() const -> GLFWwindow*
    {
      return window;
    }

  private:
    std::shared_ptr<EngineSettings> settings{ nullptr };
    float delta_time{ 0.f };
    GLFWwindow* window{ nullptr };
  };

} // namespace loki

