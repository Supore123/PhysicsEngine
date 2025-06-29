#include "render_loop.hpp"
#include "physics.hpp"
#include "particle.hpp"
#include <vector>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void renderLoop(GLFWwindow* window, GLuint gridProgram, GLuint gridVAO, int gridVertexCount, GLuint axisProgram, GLuint axisVAO, int axisVertexCount, GridRenderer& gridRenderer, PhysicsWorld& world, GLuint pointProgram) {
    // Debug: Print each frame to confirm rendering
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        printf("[renderLoop] Frame: %d, world.objects.size(): %zu\n", frameCount, world.objects.size());
        fflush(stdout);
    }
    // Ensure OpenGL state is correct for custom rendering
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
    glUseProgram(0); // Unbind any custom shader (will be set below)
    glBindVertexArray(0); // Unbind any VAO (will be set below)
    glDisable(GL_DEPTH_TEST); // 2D rendering
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLint colorLoc = glGetUniformLocation(axisProgram, "color");
    static GLint pointColorLoc = -1;
    // Draw axes in red
    glUseProgram(axisProgram);
    glUniform3f(colorLoc, 0.8f, 0.0f, 0.0f);
    gridRenderer.drawAxes(axisProgram, axisVAO, axisVertexCount);
    // Draw field (field arrows set their own color)
    gridRenderer.drawField(world, axisProgram, colorLoc);
    // Draw physics objects as circular points (point masses) in red
    glUseProgram(pointProgram);
    if (pointColorLoc == -1) pointColorLoc = glGetUniformLocation(pointProgram, "color");
    std::vector<float> points;
    std::vector<float> sizes;
    std::vector<Color3> colors;
    float minMass, maxMass;
    ParticleUtils::computeMassRange(world.objects, minMass, maxMass);
    for (const auto& obj : world.objects) {
        points.push_back(obj.x);
        points.push_back(obj.y);
        colors.push_back(ParticleUtils::getColor(reinterpret_cast<const Particle&>(obj), minMass, maxMass));
        sizes.push_back(ParticleUtils::getSize(reinterpret_cast<const Particle&>(obj)));
    }
    GLuint pointVAO, pointVBO;
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO);
    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Draw each point with its color and size
    for (size_t i = 0; i < points.size() / 2; ++i) {
        glUniform3f(pointColorLoc, colors[i].r, colors[i].g, colors[i].b);
        glPointSize(sizes[i]);
        glDrawArrays(GL_POINTS, i, 1);
    }
    glDeleteVertexArrays(1, &pointVAO);
    glDeleteBuffers(1, &pointVBO);
}
