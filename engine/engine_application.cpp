#include "engine_application.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "spdlog/spdlog.h"
#include "time/scope_timer.h"

alba::EngineApplication::~EngineApplication()
{
  if (window) {
    term();
  }
}

auto alba::EngineApplication::launch(const std::shared_ptr<EngineSettings>& _settings) -> int
{
  settings = _settings;

  if (!init()) {
    window = nullptr;
    return 1;
  }

  post_init();

  // Unbind VAO, VBO, and EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  while (!glfwWindowShouldClose(window)) {
    alba::ScopeTimer scope_timer{ delta_time };

    // Main update
    update();

    // Draw main scene
    draw();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw UI
    draw_ui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}

bool alba::EngineApplication::init()
{
  if (!glfwInit()) {
    spdlog::error("Failed to initialize GLFW!");
    return false;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Set error callback
  glfwSetErrorCallback([](int error, const char* description) {
    spdlog::error("Code: {}, description: {}", error, description);
  });

  // Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(1'920, 1'080, "Alba Engine", nullptr, nullptr);
  if (!window) {
    spdlog::error("Failed to create GLFW window!");
    glfwTerminate();
    return false;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // VSYNC
  glfwSwapInterval(1);

  if (glewInit()) {
    spdlog::error("Failed to initialize GLEW!");
    return false;
  }

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

  return true;
}

void alba::EngineApplication::term()
{
  // Some ImGui cleanups here
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  window = nullptr;
}
