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

  struct Vertex
  {
    float x, y, z;
  };

  std::vector<Vertex> generate_grid_vertices(float size, int n)
  {
    std::vector<Vertex> grid_vertices;

    // Calculate the step size between each grid cell
    float step = size / (float)n;

    // Generate vertices
    for (int i = 0; i <= n; ++i) {
      for (int j = 0; j <= n; ++j) {
        Vertex v{};
        v.x = -size / 2.0f + (float)j * step;
        v.y = 0.0f;
        v.z = -size / 2.0f + (float)i * step;
        grid_vertices.push_back(v);
      }
    }

    return grid_vertices;
  }

  std::vector<GLuint> generate_grid_indices(int n)
  {
    std::vector<GLuint> grid_indices;

    // Generate indices for each quad
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        GLuint top_left = i * (n + 1) + j;
        GLuint top_right = top_left + 1;
        GLuint bottom_left = (i + 1) * (n + 1) + j;
        GLuint bottom_right = bottom_left + 1;

        // First triangle of the quad
        grid_indices.push_back(top_left);
        grid_indices.push_back(bottom_left);
        grid_indices.push_back(top_right);

        // Second triangle of the quad
        grid_indices.push_back(top_right);
        grid_indices.push_back(bottom_left);
        grid_indices.push_back(bottom_right);
      }
    }

    return grid_indices;
  }

} // namespace kiwi

float delta_time = 0.f;

struct
{
  float distance_to_origin{ 5.0f };
  float phi{ 0.6f }, theta{ 2.7f };
} camera;

// Function to handle key presses
void key_callback(GLFWwindow* window, int key, int, int action, int)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

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

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

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

  std::vector<kiwi::Vertex> vertices = kiwi::generate_grid_vertices(1.f, 32);
  std::vector<GLuint> indices = kiwi::generate_grid_indices(32);

  // Bind VBO and copy vertices data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices.size() * sizeof(kiwi::Vertex)), vertices.data(), GL_STATIC_DRAW);

  // Bind EBO and copy indices data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

  // Set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(kiwi::Vertex), (GLvoid*)nullptr);
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

  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.f / 1080.f, 0.1f, 100.0f);

  // Needed before setting uniforms
  glUseProgram(shader_program);

  glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection"), 1, GL_FALSE, glm::value_ptr(proj));

  auto transform = glm::mat4(1.0f);
  auto last_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    const auto current_time = glfwGetTime();
    delta_time = (float)(current_time - last_time);
    last_time = current_time;

    glfwPollEvents();

    float x = camera.distance_to_origin * glm::sin(camera.phi) * glm::cos(camera.theta);
    float y = camera.distance_to_origin * glm::cos(camera.phi);
    float z = camera.distance_to_origin * glm::sin(camera.phi) * glm::sin(camera.theta);

    // Calculate view matrix
    glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    // Update uniform
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));

    static float bg_color_red = 0.144f;
    static float bg_color_green = 0.186f;
    static float bg_color_blue = 0.311f;

    glClearColor(bg_color_red, bg_color_green, bg_color_blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    glUniform1f(glGetUniformLocation(shader_program, "u_time"), (float)current_time);

    const auto model_y = (float)glm::sin(current_time);
    auto new_transform = glm::translate(transform, glm::vec3(0.f, model_y, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1, GL_FALSE, glm::value_ptr(new_transform));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);

#if 0
    new_transform = glm::translate(transform, glm::vec3(1.f, -model_y, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1, GL_FALSE, glm::value_ptr(new_transform));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
#endif

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
    ImGui::Text("Orbit Camera");
    ImGui::SliderFloat("Phi", &camera.phi, 0.0f, glm::pi<float>() * 2.f);
    ImGui::SliderFloat("Theta", &camera.theta, 0.0f, glm::pi<float>() * 2.f);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}
