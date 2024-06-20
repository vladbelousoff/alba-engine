#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <CLI/CLI.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
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
  float distance_to_origin{ 6.7f };
  float phi{ 5.182f }, theta{ 5.716f };
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

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsLight();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

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
  std::filesystem::path texture_path = alba::global::root_path / std::filesystem::path{ "data/container2.png" };
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(texture_path.string().c_str(), &width, &height, &nr_channels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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

  alba::ShaderManager::use_program(prog, [&](const alba::UniformManager& manager) {
    manager.set_uniform(alba::StringID{ "u_texture" }, 0);
  });

  auto transform = glm::mat4(1.0f);

  GLuint FBO;        // frame buffer object
  GLuint RBO;        // rendering buffer object
  GLuint texture_id; // the texture id we'll need later to create a texture

  auto create_framebuffer = [&]() {
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      spdlog::error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  };

  auto rescale_framebuffer = [&](int width, int height) {
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    alba::ShaderManager::use_program(prog, [&](const alba::UniformManager& manager) {
      manager.set_uniform(alba::StringID{ "u_projection" }, proj);
    });

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
  };

  auto bind_framebuffer = [&]() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  };

  auto unbind_framebuffer = []() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  };

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

  create_framebuffer();

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

    static float bg_color_red = 0.144f;
    static float bg_color_green = 0.186f;
    static float bg_color_blue = 0.311f;

    // glClearColor(0.f, 0.f, 0.f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.f, 0.f, 0.f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Viewport");
    ImVec2 region_size = ImGui::GetContentRegionAvail();
    glViewport(0, 0, (int)region_size.x, (int)region_size.y);
    rescale_framebuffer((int)region_size.x, (int)region_size.y);
    ImGui::Image(reinterpret_cast<ImTextureID>(texture_id), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    ImGui::SameLine(); // Move to the right of the "Viewport" child window

    ImGui::Begin("Settings");
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

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Create")) {
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
        }
        if (ImGui::MenuItem("Save as..")) {
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    bind_framebuffer();

    // glCullFace(GL_BACK);
    glViewport(0, 0, (int)region_size.x, (int)region_size.y);
    glClearColor(bg_color_red, bg_color_green, bg_color_blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto model_y = (float)glm::sin(scope_timer.get_start());
    auto new_transform = glm::translate(transform, glm::vec3(0.f, model_y, 0.f));

    alba::ShaderManager::use_program(prog, [&](const alba::UniformManager& manager) {
      manager.set_uniform(alba::StringID{ "u_view" }, view);
      manager.set_uniform(alba::StringID{ "u_model" }, new_transform);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glBindVertexArray(0);
    });

    unbind_framebuffer();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // some ImGui cleanups here
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
}
