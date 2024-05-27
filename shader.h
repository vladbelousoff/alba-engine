#pragma once

#include <cstdint>
#include <spdlog/spdlog.h>
#include <string>

#include "glm/fwd.hpp"
#include "string_manager.h"

namespace alba {

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

  private:
    uint32_t id;
  };

  struct ShaderManager
  {
    // general stuff
    static auto create_shader(const std::string& source, ShaderType type) -> ShaderHandle;
    static auto create_program(ShaderHandle vert, ShaderHandle frag) -> ProgramHandle;
    static auto use_program(ProgramHandle handle) -> void;

    // uniform setters
    static auto set_uniform(ProgramHandle handle, StringID name, float value) -> void;
    static auto set_uniform(ProgramHandle handle, StringID name, const glm::mat4& mat) -> void;
  };
} // namespace alba

template <> struct fmt::formatter<alba::ShaderType> : formatter<std::string>
{
  auto format(alba::ShaderType type, format_context& ctx)
  {
    switch (type) {
      case alba::ShaderType::VERT:
        return formatter<std::string>::format("alba::ShaderType::VERT", ctx);
      case alba::ShaderType::FRAG:
        return formatter<std::string>::format("alba::ShaderType::FRAG", ctx);
    }
    return formatter<std::string>::format("unknown", ctx);
  }
};
