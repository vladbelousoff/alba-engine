#pragma once

#include <cstdint>
#include <format>
#include <spdlog/spdlog.h>
#include <string>

#include "engine/string_manager.h"
#include "glm/fwd.hpp"

namespace loki {

  enum class ShaderType
  {
    VERT,
    FRAG,
  };

  struct ShaderManager;

  class ShaderHandle
  {
    friend struct ShaderManager;

  private:
    uint32_t id;
  };

  class ProgramHandle
  {
    friend struct ShaderManager;
    friend class UniformManager;

  private:
    uint32_t id;
  };

  class UniformManager
  {
  public:
    explicit UniformManager(ProgramHandle handle)
      : handle{ handle }
    {
    }

  public:
    auto set_uniform(StringID name, float value) const -> void;
    auto set_uniform(StringID name, const glm::mat4& mat) const -> void;

  private:
    ProgramHandle handle;
  };

  struct ShaderManager
  {
    // general stuff
    static auto create_shader(const std::string& source, ShaderType type) -> ShaderHandle;
    static auto create_program(ShaderHandle vert, ShaderHandle frag) -> ProgramHandle;
    static auto use_program(ProgramHandle handle, const std::function<void(const UniformManager& manager)>& callback) -> void;
  };
} // namespace loki

template<>
struct std::formatter<loki::ShaderType> : std::formatter<std::string>
{
  static auto format(const loki::ShaderType& type, format_context& ctx) -> decltype(ctx.out())
  {
    switch (type) {
      case loki::ShaderType::VERT:
        return format_to(ctx.out(), "loki::ShaderType::VERT");
      case loki::ShaderType::FRAG:
        return format_to(ctx.out(), "loki::ShaderType::FRAG");
    }
    return format_to(ctx.out(), "Unknown");
  }
};
