#pragma once

#include <cstdint>
#include <string>

#include <spdlog/spdlog.h>

namespace kiwi {

  enum class ShaderType
  {
    VERT,
    FRAG,
  };

  struct ShaderManager;

  struct ShaderHandle
  {
    uint32_t id;
  };

  struct ProgramHandle
  {
    uint32_t id;
  };

  struct ShaderManager
  {
    static auto create_shader(const std::string& source, ShaderType type) -> ShaderHandle;
    static auto create_program(ShaderHandle vert, ShaderHandle frag) -> ProgramHandle;
  };
} // namespace kiwi

template <> struct fmt::formatter<kiwi::ShaderType> : formatter<std::string>
{
  auto format(kiwi::ShaderType type, format_context& ctx)
  {
    switch (type) {
      case kiwi::ShaderType::VERT:
        return formatter<std::string>::format("kiwi::ShaderType::VERT", ctx);
      case kiwi::ShaderType::FRAG:
        return formatter<std::string>::format("kiwi::ShaderType::FRAG", ctx);
    }
    return formatter<std::string>::format("unknown", ctx);
  }
};
