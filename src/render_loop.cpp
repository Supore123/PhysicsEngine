#include "render_loop.hpp"
#include "physics.hpp"
#include "particle.hpp"
#include <vector>
#include <cstdio>
#include <cmath>
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
    float minMass, maxMass;
    ParticleUtils::computeMassRange(world.objects, minMass, maxMass);
    for (size_t i = 0; i < world.objects.size(); ++i) {
        const auto& obj = world.objects[i];
        Color3 color = obj.color;
        float size = obj.radius * 600.0f;
        bool customDraw = false;
        // --- Visual rotation: draw orientation marker if spinAngle is nonzero ---
        auto drawOrientationMarker = [&](const Particle& p, float markerLen, float markerWidth, const Color3& markerColor) {
            if (std::abs(p.spin) > 1e-6f || std::abs(p.spinAngle) > 1e-6f) {
                float angle = p.spinAngle;
                float x1 = p.x + std::cos(angle) * (p.radius + markerLen);
                float y1 = p.y + std::sin(angle) * (p.radius + markerLen);
                float x0 = p.x + std::cos(angle) * p.radius;
                float y0 = p.y + std::sin(angle) * p.radius;
                glUseProgram(axisProgram);
                glUniform3f(colorLoc, markerColor.r, markerColor.g, markerColor.b);
                glLineWidth(markerWidth);
                glBegin(GL_LINES);
                glVertex2f(x0, y0);
                glVertex2f(x1, y1);
                glEnd();
                glLineWidth(1.0f);
            }
        };
        // Visually distinct rendering for each type
        switch (obj.type) {
            case ObjectType::BlackHole:
                // Draw event horizon as a thick dark ring
                glUseProgram(pointProgram);
                glUniform3f(pointColorLoc, 0.1f, 0.1f, 0.1f);
                glPointSize(obj.eventHorizon * 1200.0f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                // Draw core as a smaller dark circle
                glUniform3f(pointColorLoc, 0.2f, 0.2f, 0.2f);
                glPointSize(obj.radius * 600.0f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                // Draw orientation marker for spin
                drawOrientationMarker(obj, obj.radius * 0.7f, 3.0f, {0.8f, 0.2f, 0.2f});
                customDraw = true;
                break;
            case ObjectType::Star:
                // Draw a glowing star (core + outer glow)
                glUseProgram(pointProgram);
                glUniform3f(pointColorLoc, color.r, color.g, color.b);
                glPointSize(size * 2.0f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                glUniform3f(pointColorLoc, 1.0f, 1.0f, 0.6f);
                glPointSize(size * 1.2f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                glUniform3f(pointColorLoc, color.r, color.g, color.b);
                glPointSize(size);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                drawOrientationMarker(obj, obj.radius * 1.2f, 2.0f, {1.0f, 0.8f, 0.2f});
                customDraw = true;
                break;
            case ObjectType::Planet:
                // Draw planet with a colored ring for orbit
                glUseProgram(pointProgram);
                glUniform3f(pointColorLoc, 0.7f, 0.7f, 0.7f);
                glPointSize(size * 1.5f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                glUniform3f(pointColorLoc, color.r, color.g, color.b);
                glPointSize(size);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                drawOrientationMarker(obj, obj.radius * 1.1f, 2.0f, {0.2f, 0.8f, 1.0f});
                customDraw = true;
                break;
            case ObjectType::Asteroid:
                // Draw asteroid as a small brown/gray point
                glUseProgram(pointProgram);
                glUniform3f(pointColorLoc, color.r * 0.7f, color.g * 0.6f, color.b * 0.5f);
                glPointSize(size * 0.8f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                drawOrientationMarker(obj, obj.radius * 0.7f, 1.5f, {0.7f, 0.7f, 0.7f});
                customDraw = true;
                break;
            case ObjectType::Merged:
                // Draw merged as cyan with a white outline
                glUseProgram(pointProgram);
                glUniform3f(pointColorLoc, 1.0f, 1.0f, 1.0f);
                glPointSize(size * 1.3f);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                glUniform3f(pointColorLoc, 0.2f, 0.8f, 1.0f);
                glPointSize(size);
                glBegin(GL_POINTS);
                glVertex2f(obj.x, obj.y);
                glEnd();
                drawOrientationMarker(obj, obj.radius * 1.0f, 2.0f, {0.2f, 1.0f, 1.0f});
                customDraw = true;
                break;
            default:
                break;
        }
        if (!customDraw) {
            glUseProgram(pointProgram);
            glUniform3f(pointColorLoc, color.r, color.g, color.b);
            glPointSize(size);
            glBegin(GL_POINTS);
            glVertex2f(obj.x, obj.y);
            glEnd();
            drawOrientationMarker(obj, obj.radius * 1.0f, 2.0f, {1.0f, 1.0f, 1.0f});
        }
    }
}
