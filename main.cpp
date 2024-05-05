#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

namespace kiwi {
  static void handle_glfw_error(int error, const char* description)
  {
    spdlog::error("Code: {}, description: {}", error, description);
  }

  static std::string default_shader_vert =
      "#version 330 core\n"
      "layout (location = 0) in vec3 a_position;\n"
      "uniform mat4 u_model;\n"
      "uniform mat4 u_view;\n"
      "uniform mat4 u_projection;\n"
      "void main() {\n"
      "  gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);\n"
      "}\n";

  static std::string default_shader_frag =
      "#version 330 core\n"
      "in vec2 textcoord;\n"
      "out vec4 frag_color;\n"
      "uniform float u_time;\n"
      "void main() {\n"
      "  float blue = (sin(u_time) + 1.0) * 0.5;\n"
      "  frag_color = vec4(1.0, 0.5, blue, 1.0);\n"
      "}\n";

  static uint32_t compile_shader(int shader_type, const std::string& source)
  {
    uint32_t shader_id = glCreateShader(shader_type);

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
      spdlog::info("Shader {} compiled successfully", shader_type);
    }

    return shader_id;
  }

  GLfloat vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
    -0.5f };

  GLuint indices[] = {
    // front face
    0, 1, 2, 2, 3, 0,
    // top face
    3, 2, 6, 6, 7, 3,
    // back face
    7, 6, 5, 5, 4, 7,
    // left face
    4, 0, 3, 3, 7, 4,
    // bottom face
    // Back to front now
    0, 4, 5, 5, 1, 0,
    // right face
    1, 5, 6, 6, 2, 1
  };
} // namespace kiwi

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  if (!glfwInit()) {
    spdlog::error("Failed to initialize GLFW");
    return -1;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Set error callback
  glfwSetErrorCallback(kiwi::handle_glfw_error);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow* window = glfwCreateWindow(1'920, 1'080, "Kiwi Engine", nullptr, nullptr);
  if (!window) {
    spdlog::error("Failed to create GLFW window");
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // VSYNC
  glfwSwapInterval(1);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  constexpr const char* glsl_version = "#version 330";
  ImGui_ImplOpenGL3_Init(glsl_version);

  if (glewInit()) {
    spdlog::error("Failed to initialize glew");
    return -1;
  }

  spdlog::info("Starting...");

  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  // Bind VAO
  glBindVertexArray(vao);

  // Bind VBO and copy vertices data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kiwi::vertices), kiwi::vertices, GL_STATIC_DRAW);

  // Bind EBO and copy indices data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kiwi::indices), kiwi::indices, GL_STATIC_DRAW);

  // Set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
  glEnableVertexAttribArray(0);

  // Unbind VAO, VBO, and EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  uint32_t frag_shader = kiwi::compile_shader(GL_FRAGMENT_SHADER, kiwi::default_shader_frag);
  uint32_t vert_shader = kiwi::compile_shader(GL_VERTEX_SHADER, kiwi::default_shader_vert);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, frag_shader);
  glAttachShader(shader_program, vert_shader);
  glLinkProgram(shader_program);

  float cam_pitch = 0.0f;
  float cam_yaw = 0.0f;
  float rot_scale = 0.01f;

  double prev_mouse_x = 0;
  double prev_mouse_y = 0;

  static double mouse_x = 0;
  static double mouse_y = 0;

  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.f / 1080.f, 0.1f, 100.0f);

  // Needed before setting uniforms
  glUseProgram(shader_program);

  glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection"), 1, GL_FALSE, glm::value_ptr(proj));

  static auto cursor_position_callback = [](GLFWwindow* window, double x_pos, double y_pos) {
    // Do something with the mouse position while RMB is pressed
    // Example: Print the position to the console
    spdlog::info("Mouse position: {}:{}", x_pos, y_pos);
    mouse_x = x_pos;
    mouse_y = y_pos;
  };

  static auto mouse_button_callback = [](GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      // Right mouse button pressed, start tracking mouse movement
      glfwSetCursorPosCallback(window, cursor_position_callback);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
      // Right mouse button released, stop tracking mouse movement
      glfwSetCursorPosCallback(window, nullptr);
    }
  };

  // Set up mouse button callback
  // glfwSetMouseButtonCallback(window, mouse_button_callback);

  // Set up cursor position callback
  // glfwSetCursorPosCallback(window, cursor_position_callback);

  auto transform = glm::mat4(1.0f);
  auto last_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    const auto current_time = glfwGetTime();
    auto delta_time = current_time - last_time;
    spdlog::info("delta_time: {}", delta_time);
    last_time = current_time;

    glfwPollEvents();

    {
      if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        double delta_x = mouse_x - prev_mouse_x;
        double delta_y = mouse_y - prev_mouse_y;
        cam_yaw += float(-delta_x) * rot_scale;
        cam_pitch += float(-delta_y) * rot_scale;
      }

      prev_mouse_x = mouse_x;
      prev_mouse_y = mouse_y;
    }

    glm::mat4 cam_rotation = glm::rotate(glm::mat4(1.0f), cam_pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), cam_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 cam_translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 cam_transform = cam_translation * cam_rotation;
    glm::mat4 view = glm::inverse(cam_transform);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));

    static float bg_color_red = 0.411f;
    static float bg_color_green = 0.469f;
    static float bg_color_blue = 0.588f;

    glClearColor(bg_color_red, bg_color_green, bg_color_blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    glUniform1f(glGetUniformLocation(shader_program, "u_time"), (float)current_time);

    const auto Y = (float)glm::sin(current_time);
    auto new_transform = glm::translate(transform, glm::vec3(-1.f, Y, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1, GL_FALSE, glm::value_ptr(new_transform));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    new_transform = glm::translate(transform, glm::vec3(1.f, -Y, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1, GL_FALSE, glm::value_ptr(new_transform));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");
    ImGui::Text("Background Color");
    ImGui::SliderFloat("Red", &bg_color_red, 0.0f, 1.0f);
    ImGui::SliderFloat("Green", &bg_color_green, 0.0f, 1.0f);
    ImGui::SliderFloat("Blue", &bg_color_blue, 0.0f, 1.0f);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}
