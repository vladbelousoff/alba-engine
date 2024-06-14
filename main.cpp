#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <CLI/CLI.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

#include "mt/thread_pool.h"
#include "shader.h"
#include "time/scope_timer.h"

namespace alba {
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

} // namespace alba

namespace alba {
  namespace global {
    static float delta_time = 0.f;
  }

  auto get_delta_time() -> float
  {
    return global::delta_time;
  }

  auto get_fps() -> float
  {
    return global::delta_time != 0.f ? 1.f / global::delta_time : 0.f;
  }
} // namespace alba

struct
{
  float distance_to_origin{ 3.45f };
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
  CLI::App app{ "Alba Engine" };
  argv = app.ensure_utf8(argv);

  CLI::Option* run_tests = app.add_flag("--run-tests");
  CLI11_PARSE(app, argc, argv)

  if (run_tests->count()) {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
  }

  if (!glfwInit()) {
    spdlog::error("Failed to initialize GLFW");
    return -1;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Set error callback
  glfwSetErrorCallback(alba::handle_glfw_error);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow* window = glfwCreateWindow(1'920, 1'080, app.get_name().c_str(), nullptr, nullptr);
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

  constexpr int side_n = 32;
  std::vector<alba::Vertex> vertices = alba::generate_grid_vertices(1.f, side_n);
  std::vector<GLuint> indices = alba::generate_grid_indices(side_n);

  // Bind VBO and copy vertices data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices.size() * sizeof(alba::Vertex)), vertices.data(), GL_STATIC_DRAW);

  // Bind EBO and copy indices data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

  // Set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(alba::Vertex), (GLvoid*)nullptr);
  glEnableVertexAttribArray(0);

  // Unbind VAO, VBO, and EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  auto frag = alba::ShaderManager::create_shader(alba::default_shader_frag, alba::ShaderType::FRAG);
  auto vert = alba::ShaderManager::create_shader(alba::default_shader_vert, alba::ShaderType::VERT);
  auto prog = alba::ShaderManager::create_program(vert, frag);

  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.f / 1080.f, 0.1f, 100.0f);

  // Needed before setting uniforms
  alba::ShaderManager::use_program(prog);

  alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_projection" }, proj);

  auto transform = glm::mat4(1.0f);

  alba::ThreadPool thread_pool{ 4 };

  class DefaultJob : public alba::Job
  {
  public:
    explicit DefaultJob(alba::StringID name)
        : alba::Job(name)
    {
    }

    void execute() override
    {
    }
  };

  alba::Job::SharedPtr job1 = std::make_shared<DefaultJob>(alba::StringID{ "1" });
  alba::Job::SharedPtr job2 = std::make_shared<DefaultJob>(alba::StringID{ "2" });
  alba::Job::SharedPtr job3 = std::make_shared<DefaultJob>(alba::StringID{ "3" });

  job1->add_dependency(job2);
  job2->add_dependency(job3);

  while (!glfwWindowShouldClose(window)) {
    alba::ScopeTimer scope_timer{ alba::global::delta_time };

    thread_pool.submit_job(job1);
    thread_pool.submit_job(job2);
    thread_pool.submit_job(job3);

    // Wait for all jobs to be done
    thread_pool.wait_all();
    thread_pool.reset();

    float x = camera.distance_to_origin * glm::sin(camera.phi) * glm::cos(camera.theta);
    float y = camera.distance_to_origin * glm::cos(camera.phi);
    float z = camera.distance_to_origin * glm::sin(camera.phi) * glm::sin(camera.theta);

    // Calculate view matrix
    glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    // Update uniform
    alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_view" }, view);

    static float bg_color_red = 0.144f;
    static float bg_color_green = 0.186f;
    static float bg_color_blue = 0.311f;

    glClearColor(bg_color_red, bg_color_green, bg_color_blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);

    alba::ShaderManager::use_program(prog);
    glBindVertexArray(vao);

    alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_time" }, (float)scope_timer.get_start());

    const auto model_y = (float)glm::sin(scope_timer.get_start());
    auto new_transform = glm::translate(transform, glm::vec3(0.f, model_y, 0.f));
    alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_model" }, new_transform);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Alba");
    ImGui::Text("Frame: %.3fms", alba::get_delta_time() * 1000.f);
    ImGui::Text("FPS: %.0f", alba::get_fps());
    ImGui::Text("Background Color");
    ImGui::SliderFloat("Red", &bg_color_red, 0.0f, 1.0f);
    ImGui::SliderFloat("Green", &bg_color_green, 0.0f, 1.0f);
    ImGui::SliderFloat("Blue", &bg_color_blue, 0.0f, 1.0f);
    ImGui::Text("Orbit Camera");
    ImGui::SliderFloat("Phi", &camera.phi, 0.0f, glm::pi<float>() * 2.f);
    ImGui::SliderFloat("Theta", &camera.theta, 0.0f, glm::pi<float>() * 2.f);
    ImGui::SliderFloat("Distance", &camera.distance_to_origin, 0.0f, 100.0f);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
