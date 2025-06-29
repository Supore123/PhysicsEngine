
#include "grid.hpp"
#include "physics.hpp"
#include <vector>
#include <cmath>

GridRenderer::GridRenderer(int fieldN_, float arrowScale_, float arrowAlpha_)
    : fieldN(fieldN_), arrowScale(arrowScale_), arrowAlpha(arrowAlpha_) {}

void GridRenderer::drawAxes(GLuint axisProgram, GLuint axisVAO, int axisVertexCount) {
    glUseProgram(axisProgram);
    glBindVertexArray(axisVAO);
    glDrawArrays(GL_LINES, 0, axisVertexCount);
}

void GridRenderer::drawField(const PhysicsWorld& world, GLuint axisProgram, GLint colorLoc) {
    glUseProgram(axisProgram);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float logMinG = 1e6f;
    float logMaxG = -1e6f;
    std::vector<float> gNorms;
    if (!world.objects.empty()) {
        gNorms.resize(fieldN * fieldN, 0.0f);
        // First pass: compute all gNorms
        for (int i = 0; i < fieldN; ++i) {
            for (int j = 0; j < fieldN; ++j) {
                float x = -1.0f + 2.0f * i / (fieldN - 1);
                float y = -1.0f + 2.0f * j / (fieldN - 1);
                float gx = 0.0f, gy = 0.0f;
                for (const auto& obj : world.objects) {
                    float dx = obj.x - x;
                    float dy = obj.y - y;
                    float distSq = dx * dx + dy * dy + 1e-6f;
                    float F = obj.mass / distSq;
                    gx += F * dx / std::sqrt(distSq);
                    gy += F * dy / std::sqrt(distSq);
                }
                float gNorm = std::sqrt(gx * gx + gy * gy);
                gNorms[i * fieldN + j] = gNorm;
            }
        }
        // Second pass: find min/max logG (ignore zeros)
        for (int idx = 0; idx < fieldN * fieldN; ++idx) {
            float gNorm = gNorms[idx];
            if (gNorm > 1e-8f) {
                float lg = std::log10(gNorm);
                if (lg < logMinG) logMinG = lg;
                if (lg > logMaxG) logMaxG = lg;
            }
        }
        if (logMaxG - logMinG < 1e-3f) {
            logMaxG = logMinG + 1.0f;
        }
    }
    // Only draw arrows if there are objects
    if (!world.objects.empty()) {
        for (int i = 0; i < fieldN; ++i) {
            for (int j = 0; j < fieldN; ++j) {
                float x = -1.0f + 2.0f * i / (fieldN - 1);
                float y = -1.0f + 2.0f * j / (fieldN - 1);
                float gx = 0.0f, gy = 0.0f;
                for (const auto& obj : world.objects) {
                    float dx = obj.x - x;
                    float dy = obj.y - y;
                    float distSq = dx * dx + dy * dy + 1e-6f;
                    float F = obj.mass / distSq;
                    gx += F * dx / std::sqrt(distSq);
                    gy += F * dy / std::sqrt(distSq);
                }
                float gNorm = std::sqrt(gx * gx + gy * gy);
                if (gNorm > 1e-6f) {
                    gx /= gNorm;
                    gy /= gNorm;
                }
                float logG = (gNorm > 1e-8f) ? std::log10(gNorm) : logMinG;
                float norm = (logG - logMinG) / (logMaxG - logMinG);
                if (norm < 0.0f) norm = 0.0f;
                if (norm > 1.0f) norm = 1.0f;
                // Jet-like color map, but make strong points more red
                float r, g, b;
                if (norm < 0.25f) {
                    // Blue to Cyan
                    float t = norm / 0.25f;
                    r = 0.0f;
                    g = t;
                    b = 1.0f;
                } else if (norm < 0.5f) {
                    // Cyan to Green
                    float t = (norm - 0.25f) / 0.25f;
                    r = 0.0f;
                    g = 1.0f;
                    b = 1.0f - t;
                } else if (norm < 0.65f) {
                    // Green to Yellow
                    float t = (norm - 0.5f) / 0.15f;
                    r = t;
                    g = 1.0f;
                    b = 0.0f;
                } else if (norm < 0.7f) {
                    // Yellow to Red (very sharp)
                    float t = (norm - 0.65f) / 0.05f;
                    r = 1.0f;
                    g = 1.0f - t;
                    b = 0.0f;
                } else {
                    // Strongest: pure red (wider region)
                    r = 1.0f;
                    g = 0.0f;
                    b = 0.0f;
                }
                float x2 = x + gx * arrowScale;
                float y2 = y + gy * arrowScale;
                std::vector<float> arrow = {x, y, x2, y2};
                GLuint arrowVAO, arrowVBO;
                glGenVertexArrays(1, &arrowVAO);
                glGenBuffers(1, &arrowVBO);
                glBindVertexArray(arrowVAO);
                glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
                glBufferData(GL_ARRAY_BUFFER, arrow.size() * sizeof(float), arrow.data(), GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glUniform3f(colorLoc, r, g, b);
                glDrawArrays(GL_LINES, 0, 2);
                // Arrow head
                float ah = 0.02f;
                float ax = x2 - gx * ah, ay = y2 - gy * ah;
                float perpX = -gy, perpY = gx;
                std::vector<float> head1 = {x2, y2, ax + perpX * ah, ay + perpY * ah};
                std::vector<float> head2 = {x2, y2, ax - perpX * ah, ay - perpY * ah};
                GLuint h1VAO, h1VBO, h2VAO, h2VBO;
                glGenVertexArrays(1, &h1VAO);
                glGenBuffers(1, &h1VBO);
                glBindVertexArray(h1VAO);
                glBindBuffer(GL_ARRAY_BUFFER, h1VBO);
                glBufferData(GL_ARRAY_BUFFER, head1.size() * sizeof(float), head1.data(), GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glUniform3f(colorLoc, r, g, b);
                glDrawArrays(GL_LINES, 0, 2);
                glDeleteVertexArrays(1, &h1VAO);
                glDeleteBuffers(1, &h1VBO);
                glGenVertexArrays(1, &h2VAO);
                glGenBuffers(1, &h2VBO);
                glBindVertexArray(h2VAO);
                glBindBuffer(GL_ARRAY_BUFFER, h2VBO);
                glBufferData(GL_ARRAY_BUFFER, head2.size() * sizeof(float), head2.data(), GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glUniform3f(colorLoc, r, g, b);
                glDrawArrays(GL_LINES, 0, 2);
                glDeleteVertexArrays(1, &h2VAO);
                glDeleteBuffers(1, &h2VBO);
                glDeleteVertexArrays(1, &arrowVAO);
                glDeleteBuffers(1, &arrowVBO);
            }
        }
    }
    glDisable(GL_BLEND);
    // Restore OpenGL color state to white after drawing field arrows
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
}
