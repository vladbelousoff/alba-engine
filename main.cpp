#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <iostream>

namespace kiwi {
    static void ErrorCallstack(int error, const char *description) {
        std::cerr << "Error: " << error << " " << description << std::endl;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Set error callback
    glfwSetErrorCallback(kiwi::ErrorCallstack);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Terrain Generator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
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
    ImGuiIO &IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    constexpr const char *GLSLVersion = "#version 330";
    ImGui_ImplOpenGL3_Init(GLSLVersion);

    if (glewInit()) {
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        static float backgroundRed = 0.411f;
        static float backgroundGreen = 0.469f;
        static float backgroundBlue = 0.588f;

        glClearColor(backgroundRed, backgroundGreen, backgroundBlue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        ImGui::Text("Background Color");
        ImGui::SliderFloat("Red", &backgroundRed, 0.0f, 1.0f);
        ImGui::SliderFloat("Green", &backgroundGreen, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue", &backgroundBlue, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
