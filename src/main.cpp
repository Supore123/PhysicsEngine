#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "physics.hpp"
#include "grid.hpp"
#include "render_utils.hpp"
#include "render_loop.hpp"
#include "camera.hpp"
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
// Load shaders from external files for axis, grid, and points
#include <fstream>
#include <sstream>
std::string loadShaderSource(const char* path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ...existing code...

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
    // Camera state
    Camera3D camera;
    camera.distance = 8.0f;
    camera.yaw = 0.0f;
    camera.pitch = 0.0f;
    camera.targetX = 0.0f;
    camera.targetY = 0.0f;
    camera.targetZ = 0.0f;
    camera.fov = 45.0f;
    std::cout << "[DEBUG] Camera initial position: distance=" << camera.distance << ", yaw=" << camera.yaw << ", pitch=" << camera.pitch << std::endl;
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

    // 3D Grid
    std::vector<float> gridVertices;
    const int N = 20; // Number of lines per axis
    const float gridSize = 10.0f; // Half-extent of grid in each direction
    // Generate 3D grid lines (XY, XZ, YZ planes)
    for (int i = -N; i <= N; ++i) {
        float t = (float)i / N * gridSize;
        // XY plane (Z=0)
        gridVertices.push_back(-gridSize); gridVertices.push_back(t); gridVertices.push_back(0.0f);
        gridVertices.push_back(gridSize);  gridVertices.push_back(t); gridVertices.push_back(0.0f);
        gridVertices.push_back(t); gridVertices.push_back(-gridSize); gridVertices.push_back(0.0f);
        gridVertices.push_back(t); gridVertices.push_back(gridSize);  gridVertices.push_back(0.0f);
        // XZ plane (Y=0)
        gridVertices.push_back(-gridSize); gridVertices.push_back(0.0f); gridVertices.push_back(t);
        gridVertices.push_back(gridSize);  gridVertices.push_back(0.0f); gridVertices.push_back(t);
        gridVertices.push_back(t); gridVertices.push_back(0.0f); gridVertices.push_back(-gridSize);
        gridVertices.push_back(t); gridVertices.push_back(0.0f); gridVertices.push_back(gridSize);
        // YZ plane (X=0)
        gridVertices.push_back(0.0f); gridVertices.push_back(-gridSize); gridVertices.push_back(t);
        gridVertices.push_back(0.0f); gridVertices.push_back(gridSize);  gridVertices.push_back(t);
        gridVertices.push_back(0.0f); gridVertices.push_back(t); gridVertices.push_back(-gridSize);
        gridVertices.push_back(0.0f); gridVertices.push_back(t); gridVertices.push_back(gridSize);
    }
    GLuint gridVAO, gridVBO;
    setupVAOandVBO(gridVAO, gridVBO, gridVertices);
    std::string gridVertSrc = loadShaderSource("grid.vert");
    std::string gridFragSrc = loadShaderSource("grid.frag");
    GLuint gridProgram = createShaderProgram(gridVertSrc.c_str(), gridFragSrc.c_str());

    // Axes
    std::vector<float> axisVertices, axisColors;
    // X axis (red)
    axisVertices.push_back(-5.0f); axisVertices.push_back(0.0f); axisVertices.push_back(0.0f);
    axisVertices.push_back( 5.0f); axisVertices.push_back(0.0f); axisVertices.push_back(0.0f);
    axisColors.push_back(1.0f); axisColors.push_back(0.0f); axisColors.push_back(0.0f);
    axisColors.push_back(1.0f); axisColors.push_back(0.0f); axisColors.push_back(0.0f);
    // Y axis (green)
    axisVertices.push_back(0.0f); axisVertices.push_back(-5.0f); axisVertices.push_back(0.0f);
    axisVertices.push_back(0.0f); axisVertices.push_back( 5.0f); axisVertices.push_back(0.0f);
    axisColors.push_back(0.0f); axisColors.push_back(1.0f); axisColors.push_back(0.0f);
    axisColors.push_back(0.0f); axisColors.push_back(1.0f); axisColors.push_back(0.0f);
    // Z axis (blue)
    axisVertices.push_back(0.0f); axisVertices.push_back(0.0f); axisVertices.push_back(-5.0f);
    axisVertices.push_back(0.0f); axisVertices.push_back(0.0f); axisVertices.push_back( 5.0f);
    axisColors.push_back(0.0f); axisColors.push_back(0.0f); axisColors.push_back(1.0f);
    axisColors.push_back(0.0f); axisColors.push_back(0.0f); axisColors.push_back(1.0f);
    GLuint axisVAO, axisVBO[2];
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(2, axisVBO);
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, axisColors.size() * sizeof(float), axisColors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    std::string axisVertSrc = loadShaderSource("axis.vert");
    std::string axisFragSrc = loadShaderSource("axis.frag");
    GLuint axisProgram = createShaderProgram(axisVertSrc.c_str(), axisFragSrc.c_str());

    // Points (for objects)
    // 3D Points (for objects, fallback to GL_POINTS if spheres fail)
    std::string pointVertSrc = loadShaderSource("point.vert");
    std::string pointFragSrc = loadShaderSource("point.frag");
    GLuint pointProgram = createShaderProgram(pointVertSrc.c_str(), pointFragSrc.c_str());

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

    // Ensure vector field is on by default
    uiState.showField3D = true;

    // Main loop with ImGui and custom rendering
    // Camera mouse state
    static double lastMouseX = 0, lastMouseY = 0;
    static bool draggingOrbit = false, draggingPan = false;
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

        // --- Camera controls ---
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        if (!io.WantCaptureMouse) {
            if (leftDown && !draggingOrbit) { draggingOrbit = true; lastMouseX = mouseX; lastMouseY = mouseY; }
            if (!leftDown) draggingOrbit = false;
            if (rightDown && !draggingPan) { draggingPan = true; lastMouseX = mouseX; lastMouseY = mouseY; }
            if (!rightDown) draggingPan = false;
            if (draggingOrbit) {
                float dx = float(mouseX - lastMouseX);
                float dy = float(mouseY - lastMouseY);
                camera.yaw   += dx * 0.01f;
                camera.pitch += dy * 0.01f;
                if (camera.pitch > 1.5f) camera.pitch = 1.5f;
                if (camera.pitch < -1.5f) camera.pitch = -1.5f;
                lastMouseX = mouseX; lastMouseY = mouseY;
            }
            if (draggingPan) {
                float dx = float(mouseX - lastMouseX) * 0.002f * camera.distance;
                float dy = float(mouseY - lastMouseY) * 0.002f * camera.distance;
                // Pan in camera's local X/Y
                camera.targetX -= dx * cosf(camera.yaw) + dy * sinf(camera.pitch) * sinf(camera.yaw);
                camera.targetY += dy * cosf(camera.pitch);
                camera.targetZ += dx * sinf(camera.yaw) - dy * sinf(camera.pitch) * cosf(camera.yaw);
                lastMouseX = mouseX; lastMouseY = mouseY;
            }
            // Mouse wheel for zoom
            if (io.MouseWheel != 0.0f) {
                camera.distance -= io.MouseWheel * 0.2f;
                if (camera.distance < camera.minDistance) camera.distance = camera.minDistance;
                if (camera.distance > camera.maxDistance) camera.distance = camera.maxDistance;
            }
        }

        // Custom keyboard input (add/remove objects, pause, etc.)
        if (!io.WantCaptureKeyboard) {
            // Reset simulation with 'R'
            static bool prevRDown = false;
            bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
            if (rDown && !prevRDown) {
                // Reset world and UI state
                world.objects.clear();
                // world.reset(); // No reset() method; clear is enough
                uiState = UIState();
                uiState.showField3D = true;
            }
            prevRDown = rDown;
            // Zoom with + and - keys
            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
                camera.distance -= 0.1f * camera.distance;
                if (camera.distance < camera.minDistance) camera.distance = camera.minDistance;
            }
            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
                camera.distance += 0.1f * camera.distance;
                if (camera.distance > camera.maxDistance) camera.distance = camera.maxDistance;
            }
            // WASD camera movement (move target in camera's local X/Z plane)
            float camMoveSpeed = 0.025f * camera.distance; // Less sensitive
            bool moved = false;
            float forwardX = sinf(camera.yaw);
            float forwardZ = -cosf(camera.yaw);
            float rightX = cosf(camera.yaw);
            float rightZ = sinf(camera.yaw);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                camera.targetX += forwardX * camMoveSpeed;
                camera.targetZ += forwardZ * camMoveSpeed;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                camera.targetX -= forwardX * camMoveSpeed;
                camera.targetZ -= forwardZ * camMoveSpeed;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                camera.targetX -= rightX * camMoveSpeed;
                camera.targetZ += rightZ * camMoveSpeed;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                camera.targetX += rightX * camMoveSpeed;
                camera.targetZ -= rightZ * camMoveSpeed;
                moved = true;
            }
            // Up/Down for Y axis
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                camera.targetY += camMoveSpeed;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                camera.targetY -= camMoveSpeed;
                moved = true;
            }
            // Arrow keys for camera rotation (less sensitive)
            float rotSpeed = 0.01f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                camera.yaw -= rotSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                camera.yaw += rotSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                camera.pitch += rotSpeed;
                if (camera.pitch > 1.5f) camera.pitch = 1.5f;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                camera.pitch -= rotSpeed;
                if (camera.pitch < -1.5f) camera.pitch = -1.5f;
            }
            // Add object with 'M' (generate at camera target, with random Z and color)
            if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
                // Place sphere at camera target, with random Z offset and random color
                float x = camera.targetX;
                float y = camera.targetY;
                float z = camera.targetZ;
                // Add a small random Z offset for variety
                float zOffset = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
                z += zOffset * 0.5f;
                float mass = 1.0f;
                float radius = 0.25f; // Larger default for visibility
                // Optionally, randomize color or velocity here if supported by Particle struct
                gWorld->addObject({x, y, z, 0.0f, radius, mass, 0.0f, false});
                std::cout << "[DEBUG] Added sphere at (" << x << ", " << y << ", " << z << ") radius=" << radius << std::endl;
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
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Pass number of grid lines as (number of vertices / 2) for 3D lines
        // Render all objects as 3D points if sphere rendering fails
        renderLoop(window, gridProgram, gridVAO, gridVertices.size() / 3, axisProgram, axisVAO, axisVertices.size() / 2, gridRenderer, world, pointProgram, camera, uiState);

        // Draw ImGui UI (just widgets, not rendering)
        // Camera UI
        ImGui::Begin("Camera Controls");
        ImGui::SliderFloat("Distance", &camera.distance, camera.minDistance, camera.maxDistance);
        ImGui::SliderFloat("Yaw", &camera.yaw, -3.14f, 3.14f);
        ImGui::SliderFloat("Pitch", &camera.pitch, -1.5f, 1.5f);
        ImGui::InputFloat3("Target", &camera.targetX);
        if (ImGui::Button("Reset Camera")) {
            camera.distance = 8.0f; camera.yaw = 0.0f; camera.pitch = 0.0f;
            camera.targetX = camera.targetY = camera.targetZ = 0.0f;
        }
        ImGui::End();
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
    glDeleteBuffers(2, axisVBO);
    glDeleteProgram(gridProgram);
    glDeleteProgram(axisProgram);
    glDeleteProgram(pointProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
