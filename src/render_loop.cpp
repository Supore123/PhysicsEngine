#include "render_loop.hpp"
#include "physics.hpp"
#include "particle.hpp"
#include <vector>
#include <cstdio>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "glm_compat.hpp"
#include "ui.hpp"
#include "render_utils.hpp"


// Global pointer for field computation
static PhysicsWorld* g_world = nullptr;
extern "C" void computeFieldAtPoint(float x, float y, float z, float& vx, float& vy, float& vz) {
    vx = 0.0f; vy = 0.0f; vz = 0.0f;
    if (!g_world) return;
    for (const auto& obj : g_world->objects) {
        float dx = obj.x - x;
        float dy = obj.y - y;
        float dz = obj.z - z;
        float distSq = dx*dx + dy*dy + dz*dz + 1e-4f;
        float dist = sqrtf(distSq);
        float strength = obj.mass / distSq;
        vx += strength * dx / dist;
        vy += strength * dy / dist;
        vz += strength * dz / dist;
    }
}

void renderLoop(GLFWwindow* window, GLuint gridProgram, GLuint gridVAO, int gridVertexCount, GLuint axisProgram, GLuint axisVAO, int axisVertexCount, GridRenderer& gridRenderer, PhysicsWorld& world, GLuint pointProgram, const Camera3D& camera, UIState& uiState) {
    g_world = &world;
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
    glEnable(GL_DEPTH_TEST); // Enable depth for 3D
    glEnable(GL_CULL_FACE); // Enable culling for 3D
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Camera setup (modern OpenGL) ---
    // Compute view and projection matrices (right-handed, Y up)
    float aspect = (float)fbWidth / (float)fbHeight;
    float camX, camY, camZ;
    camera.getPosition(camX, camY, camZ);
    vec3 eye(camX, camY, camZ);
    vec3 center(camera.targetX, camera.targetY, camera.targetZ);
    vec3 up(0.0f, 1.0f, 0.0f);
    mat4 view = lookAt(eye, center, up);
    mat4 proj = perspective(camera.fov, aspect, 0.01f, 100.0f);

    // Draw 3D axis indicator at origin using modern OpenGL (axis VAO/VBO, axisProgram)
    glUseProgram(axisProgram);
    GLint viewLoc = glGetUniformLocation(axisProgram, "view");
    GLint projLoc = glGetUniformLocation(axisProgram, "proj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.m[0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj.m[0]);
    glBindVertexArray(axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
    // Draw grid and field using modern OpenGL (if needed)
    // Optionally, update view/proj matrices if camera moved (already done above)
    // Draw vector field using the computeFieldAtPoint function
    if (uiState.showField3D) {
        glUseProgram(axisProgram);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.m[0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj.m[0]);
        drawVectorField3D(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 0.4f, computeFieldAtPoint);
        glUseProgram(0);
    }

    // --- Draw particles/objects as GL_POINTS ---
    if (!world.objects.empty()) {
        glUseProgram(pointProgram);
        GLint viewLocPt = glGetUniformLocation(pointProgram, "view");
        GLint projLocPt = glGetUniformLocation(pointProgram, "proj");
        glUniformMatrix4fv(viewLocPt, 1, GL_FALSE, &view.m[0]);
        glUniformMatrix4fv(projLocPt, 1, GL_FALSE, &proj.m[0]);
        // Prepare positions, radii, and colors, scale radii for visual clarity
        std::vector<float> positions;
        std::vector<float> radii;
        std::vector<float> colors;
        for (const auto& obj : world.objects) {
            positions.push_back(obj.x);
            positions.push_back(obj.y);
            positions.push_back(obj.z);
            float visualScale = 1.0f;
            switch (obj.type) {
                case ObjectType::Star:
                    // Make the sun (star at origin) much larger for visibility
                    if (std::abs(obj.x) < 1e-4f && std::abs(obj.y) < 1e-4f && std::abs(obj.z) < 1e-4f) {
                        visualScale = 7.0f;
                    } else {
                        visualScale = 2.2f;
                    }
                    break;
                case ObjectType::BlackHole: visualScale = 2.5f; break;
                case ObjectType::Planet: visualScale = 1.5f; break;
                case ObjectType::Asteroid: visualScale = 1.1f; break;
                default: visualScale = 1.0f; break;
            }
            radii.push_back(obj.radius * visualScale);
            colors.push_back(obj.color.r);
            colors.push_back(obj.color.g);
            colors.push_back(obj.color.b);
        }
        GLuint VAO, VBO[3];
        glGenVertexArrays(1, &VAO);
        glGenBuffers(3, VBO);
        glBindVertexArray(VAO);
        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        // Radius attribute (location = 1)
        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, radii.size() * sizeof(float), radii.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        // Color attribute (location = 2)
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
        glDrawArrays(GL_POINTS, 0, world.objects.size());
        glBindVertexArray(0);
        glDeleteBuffers(3, VBO);
        glDeleteVertexArrays(1, &VAO);
        glUseProgram(0);
    }
    // End of renderLoop
}
