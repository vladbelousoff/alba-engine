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

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

namespace alba {
  static void handle_glfw_error(int error, const char* description)
  {
    spdlog::error("Code: {}, description: {}", error, description);
  }

  static std::string default_shader_vert =
      "#version 330 core\n"
      "layout (location = 0) in vec3 a_position;\n"
      "layout (location = 1) in vec2 a_texcoord;\n"
      "uniform mat4 u_model;\n"
      "uniform mat4 u_view;\n"
      "uniform mat4 u_projection;\n"
      "out vec3 color;\n"
      "out vec2 texcoord;\n"
      "void main() {\n"
      "  gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);\n"
      "  texcoord = a_texcoord;\n"
      "}\n";

  static std::string default_shader_frag =
      "#version 330 core\n"
      "out vec4 frag_color;\n"
      "in vec2 texcoord;\n"
      "uniform sampler2D u_texture;\n"
      "void main() {\n"
      "  frag_color = texture(u_texture, texcoord);\n"
      "}\n";

  float vertices[] = { -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f,
    1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,
    0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f,
    0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f,
    1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f,
    -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f,
    0.0f, 1.0f };

} // namespace alba

namespace alba {
  namespace global {
    static float delta_time = 0.f;
    static std::filesystem::path root_path;
  } // namespace global

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
  float distance_to_origin{ 5.6f };
  float phi{ 4.2f }, theta{ 1.7f };
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
  app.add_option("--root", alba::global::root_path);
  CLI11_PARSE(app, argc, argv)

  spdlog::info("Root: {}", absolute(alba::global::root_path).string());

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

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  // Bind VAO
  glBindVertexArray(vao);

  // constexpr int side_n = 32;
  // std::vector<alba::Vertex> vertices = alba::generate_grid_vertices(1.f, side_n);
  // std::vector<GLuint> indices = alba::generate_grid_indices(side_n);

  // Bind VBO and copy vertices data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(alba::vertices), alba::vertices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
  glEnableVertexAttribArray(0);
  // texture attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  spdlog::info("Loading textures...");

  std::uint32_t texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nr_channels;
  std::filesystem::path texture_path = alba::global::root_path / std::filesystem::path{ "data/wall.jpg" };
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(texture_path.string().c_str(), &width, &height, &nr_channels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  } else {
    spdlog::error("Failed to load texture");
  }

  // Unbind VAO, VBO, and EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);

  auto frag = alba::ShaderManager::create_shader(alba::default_shader_frag, alba::ShaderType::FRAG);
  auto vert = alba::ShaderManager::create_shader(alba::default_shader_vert, alba::ShaderType::VERT);
  auto prog = alba::ShaderManager::create_program(vert, frag);

  glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.f / 1080.f, 0.1f, 100.0f);

  // Needed before setting uniforms
  alba::ShaderManager::use_program(prog);
  alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_projection" }, proj);
  alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_texture" }, 0);

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

    // thread_pool.submit_job(job1);
    // thread_pool.submit_job(job2);
    // thread_pool.submit_job(job3);

    // Wait for all jobs to be done
    // thread_pool.wait_all();
    // thread_pool.reset();

    float x = camera.distance_to_origin * glm::sin(camera.phi) * glm::cos(camera.theta);
    float y = camera.distance_to_origin * glm::cos(camera.phi);
    float z = camera.distance_to_origin * glm::sin(camera.phi) * glm::sin(camera.theta);

    // Calculate view matrix
    glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    alba::ShaderManager::use_program(prog);

    // Update uniform
    alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_view" }, view);

    static float bg_color_red = 0.144f;
    static float bg_color_green = 0.186f;
    static float bg_color_blue = 0.311f;

    glClearColor(bg_color_red, bg_color_green, bg_color_blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glCullFace(GL_BACK);

    // alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_time" }, (float)scope_timer.get_start());

    const auto model_y = (float)glm::sin(scope_timer.get_start());
    auto new_transform = glm::translate(transform, glm::vec3(0.f, model_y, 0.f));
    alba::ShaderManager::set_uniform(prog, alba::StringID{ "u_model" }, new_transform);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
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
