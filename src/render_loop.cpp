#include "render_loop.hpp"
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void renderLoop(GLFWwindow* window, GLuint gridProgram, GLuint gridVAO, int gridVertexCount, GLuint axisProgram, GLuint axisVAO, int axisVertexCount, GridRenderer& gridRenderer, PhysicsWorld& world) {
    double lastTime = glfwGetTime();
    GLint colorLoc = glGetUniformLocation(axisProgram, "color");
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        if (!world.objects.empty()) {
            world.step(dt);
        }
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Draw axes in red
        glUseProgram(axisProgram);
        glUniform3f(colorLoc, 0.8f, 0.0f, 0.0f);
        gridRenderer.drawAxes(axisProgram, axisVAO, axisVertexCount);
        // Draw field (field arrows set their own color)
        gridRenderer.drawField(world, axisProgram, colorLoc);
        // Draw physics objects as points (point masses) in red
        glUseProgram(axisProgram);
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
        glPointSize(10.0f);
        std::vector<float> points;
        for (const auto& obj : world.objects) {
            points.push_back(obj.x);
            points.push_back(obj.y);
        }
        GLuint pointVAO, pointVBO;
        glGenVertexArrays(1, &pointVAO);
        glGenBuffers(1, &pointVBO);
        glBindVertexArray(pointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_POINTS, 0, points.size() / 2);
        glDeleteVertexArrays(1, &pointVAO);
        glDeleteBuffers(1, &pointVBO);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
