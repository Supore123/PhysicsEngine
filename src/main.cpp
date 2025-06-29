// ...existing code...
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "physics.hpp"
#include "grid.hpp"
#include "render_utils.hpp"
#include "render_loop.hpp"
// Shader sources
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
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

// Utility and VAO/VBO functions moved to render_utils.cpp/hpp

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// Mouse callback for spawning/removing objects
PhysicsWorld* gWorld = nullptr;
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!gWorld) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        int width, height;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);
        // Convert to normalized device coordinates (-1 to 1)
        float x = (float)((xpos / width) * 2.0 - 1.0);
        float y = (float)(1.0 - (ypos / height) * 2.0);
        // Spawn with test mass, zero velocity
        float mass = 1.0f;
        float radius = 0.05f;
        gWorld->addObject({x, y, 0.0f, 0.0f, radius, mass, 0.0f, false});
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Remove last non-static object if any
        for (int i = static_cast<int>(gWorld->objects.size()) - 1; i >= 0; --i) {
            if (!gWorld->objects[i].isStatic) {
                gWorld->objects.erase(gWorld->objects.begin() + i);
                break;
            }
        }
    }
}

// Main render loosrc/grid.cpp src/main.cpp src/physics.cpp src/render_loop.cpp src/render_utils.cppp moved to render_loop.cpp/hpp

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Grid", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);

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

    // Grid renderer
    #include "grid.hpp"
    GridRenderer gridRenderer;

    // --- Physics test integration ---
    PhysicsWorld world;
    world.gravity = 0.0f;
    gWorld = &world;
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Set default color for axes (red)
    glUseProgram(axisProgram);
    GLint colorLoc = glGetUniformLocation(axisProgram, "color");
    glUniform3f(colorLoc, 0.8f, 0.0f, 0.0f);

    renderLoop(window, gridProgram, gridVAO, gridVertices.size() / 2, axisProgram, axisVAO, axisVertices.size() / 2, gridRenderer, world);

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(gridProgram);
    glDeleteProgram(axisProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
