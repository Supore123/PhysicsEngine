#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "physics.hpp"
#include "grid.hpp"
#include "render_utils.hpp"
#include "render_loop.hpp"
#include "ui.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ui.hpp"
// Callback to resize viewport with window
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
// Shader sources
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        gl_PointSize = 24.0;
    }
 )";

const char* gridFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.2, 0.2, 0.2, 1.0);
    }
 )";

const char* axisFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
 )";

// Add a fragment shader for square points
const char* pointFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
 )";

// Utility and VAO/VBO functions moved to render_utils.cpp/hpp

// Mouse callback for spawning/removing objects
PhysicsWorld* gWorld = nullptr;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (key == GLFW_KEY_M && gWorld) {
            double xpos, ypos;
            int width, height;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);
            float x = (float)((xpos / width) * 2.0 - 1.0);
            float y = (float)(1.0 - (ypos / height) * 2.0);
            float mass = 1.0f;
            float radius = 0.02f;
            gWorld->addObject({x, y, 0.0f, 0.0f, radius, mass, 0.0f, false});
        }
        if (key == GLFW_KEY_BACKSPACE && gWorld) {
            for (int i = static_cast<int>(gWorld->objects.size()) - 1; i >= 0; --i) {
                if (!gWorld->objects[i].isStatic) {
                    gWorld->objects.erase(gWorld->objects.begin() + i);
                    break;
                }
            }
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!gWorld) return;
    // Mouse no longer spawns or removes objects
}

// Main render loosrc/grid.cpp src/main.cpp src/physics.cpp src/render_loop.cpp src/render_utils.cppp moved to render_loop.cpp/hpp

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    // Create a fullscreen window on the primary monitor
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "OpenGL Grid", primary, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Set viewport to window size at startup
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    // Respond to window resizes
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // Enable keyboard navigation and mouse for ImGui
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplOpenGL3_Init("#version 330");
    // Debug: Print ImGui version and context pointer
    std::cout << "ImGui Version: " << IMGUI_VERSION << std::endl;
    std::cout << "ImGui Context: " << ImGui::GetCurrentContext() << std::endl;
    // Debug: Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Do not override ImGui's input callbacks; use input capture flags in main loop

    // Grid
    std::vector<float> gridVertices;
    const int N = 20;
    generateGridVertices(gridVertices, N);
    GLuint gridVAO, gridVBO;
    setupVAOandVBO(gridVAO, gridVBO, gridVertices);
    GLuint gridProgram = createShaderProgram(vertexShaderSource, gridFragmentShaderSource);

    // Axes
    std::vector<float> axisVertices;
    generateAxisVertices(axisVertices);
    GLuint axisVAO, axisVBO;
    setupVAOandVBO(axisVAO, axisVBO, axisVertices);
    GLuint axisProgram = createShaderProgram(vertexShaderSource, axisFragmentShaderSource);

    // Points (for objects)
    GLuint pointProgram = createShaderProgram(vertexShaderSource, pointFragmentShaderSource);

    // Grid renderer
    GridRenderer gridRenderer;

    // --- Physics test integration ---
    PhysicsWorld world;
    world.gravity = 0.0f;
    gWorld = &world;
    // Do not override ImGui's input callbacks; use input capture flags in main loop

    // Set default color for axes (red)
    glUseProgram(axisProgram);
    GLint colorLoc = glGetUniformLocation(axisProgram, "color");
    glUniform3f(colorLoc, 0.8f, 0.0f, 0.0f);

    // UI state
    UIState uiState;

    // Main loop with ImGui and custom rendering
    while (!glfwWindowShouldClose(window)) {
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Physics step always runs if not paused, scaled by UI timeScale
        ImGuiIO& io = ImGui::GetIO();
        if (!uiState.paused && !world.objects.empty()) {
            world.gravity = uiState.gravity;
            world.step((1.0f / 60.0f) * uiState.timeScale); // Scaled timestep
        }

        // Custom keyboard input (add/remove objects, pause, etc.)
        if (!io.WantCaptureKeyboard) {
            // Add object with 'M'
            if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
                double xpos, ypos;
                int width, height;
                glfwGetCursorPos(window, &xpos, &ypos);
                glfwGetWindowSize(window, &width, &height);
                float x = (float)((xpos / width) * 2.0 - 1.0);
                float y = (float)(1.0 - (ypos / height) * 2.0);
                float mass = 1.0f;
                float radius = 0.02f;
                gWorld->addObject({x, y, 0.0f, 0.0f, radius, mass, 0.0f, false});
            }
            // Remove last non-static object with Backspace
            if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
                for (int i = static_cast<int>(gWorld->objects.size()) - 1; i >= 0; --i) {
                    if (!gWorld->objects[i].isStatic) {
                        gWorld->objects.erase(gWorld->objects.begin() + i);
                        break;
                    }
                }
            }
            // Pause/unpause with 'P' (only on key press, not hold)
            static bool prevPDown = false;
            bool pDown = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
            if (pDown && !prevPDown) {
                uiState.paused = !uiState.paused;
            }
            prevPDown = pDown;
            // Quit with Escape
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderLoop(window, gridProgram, gridVAO, gridVertices.size() / 2, axisProgram, axisVAO, axisVertices.size() / 2, gridRenderer, world, pointProgram);

        // Draw ImGui UI (just widgets, not rendering)
        drawUI(uiState, window, &world);

        // --- Ensure OpenGL state for ImGui ---
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_SCISSOR_TEST); // ImGui will enable if needed
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
        glUseProgram(0); // Unbind any custom shader
        glBindVertexArray(0); // Unbind any VAO
        glDisable(GL_DEPTH_TEST); // ImGui does not use depth
        glDisable(GL_CULL_FACE); // ImGui does not use face culling
        // ---

        // Render ImGui on top of everything
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(gridProgram);
    glDeleteProgram(axisProgram);
    glDeleteProgram(pointProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
