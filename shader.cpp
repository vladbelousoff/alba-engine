#include "shader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

auto kiwi::ShaderManager::create_shader(const std::string& source, ShaderType type) -> kiwi::ShaderHandle
{
  uint32_t shader_id;
  switch (type) {
    case ShaderType::VERT:
      shader_id = glCreateShader(GL_VERTEX_SHADER);
      break;
    case ShaderType::FRAG:
      shader_id = glCreateShader(GL_FRAGMENT_SHADER);
      break;
  }

  const char* source_data = source.data();
  glShaderSource(shader_id, 1, (const GLchar**)&source_data, nullptr);
  glCompileShader(shader_id);

  GLint shader_compiled;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compiled);

  if (!shader_compiled) {
    int log_msg_len;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_msg_len);
    std::vector<char> shader_log(log_msg_len + 1);
    glGetShaderInfoLog(shader_id, log_msg_len, &log_msg_len, shader_log.data());
    shader_log.at(log_msg_len) = 0;
    spdlog::error("Shader compilation failed, the compile log: {}", shader_log.data());
  } else {
    spdlog::info("Shader {} compiled successfully", type);
  }

  kiwi::ShaderHandle handle{};
  handle.id = shader_id;

  return handle;
}

auto kiwi::ShaderManager::create_program(kiwi::ShaderHandle vert, kiwi::ShaderHandle frag) -> kiwi::ProgramHandle
{
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vert.id);
  glAttachShader(shader_program, frag.id);
  glLinkProgram(shader_program);

  kiwi::ProgramHandle handle{};
  handle.id = shader_program;
  
  return handle;
}
