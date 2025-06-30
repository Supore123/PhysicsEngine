// Only include <GL/glew.h> and <cmath> once, and after header
#include "render_utils.hpp"
#include <GL/glew.h>
#include <vector>
#include <cmath>
// Draw a 3D vector field (arrows) across the entire axis-aligned box
// vecFunc(x, y, z, out_vx, out_vy, out_vz) should fill in the vector at (x, y, z)
void drawVectorField3D(float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float spacing, void(*vecFunc)(float,float,float,float&,float&,float&)) {
    // Modern OpenGL: build a vertex array for all arrows (lines + 3D cone arrowheads)
    std::vector<float> vertices;
    std::vector<float> colors;
    float minStrength = 1e10f, maxStrength = 0.0f;
    std::vector<float> strengths;
    // First pass: find min/max field strength for color mapping
    for (float x = minX; x <= maxX; x += spacing) {
        for (float y = minY; y <= maxY; y += spacing) {
            for (float z = minZ; z <= maxZ; z += spacing) {
                float vx, vy, vz;
                vecFunc(x, y, z, vx, vy, vz);
                float len = sqrtf(vx*vx + vy*vy + vz*vz);
                strengths.push_back(len);
                if (len < minStrength) minStrength = len;
                if (len > maxStrength) maxStrength = len;
            }
        }
    }
    // Second pass: generate geometry and color
    int idx = 0;
    const int coneSegments = 10;
    for (float x = minX; x <= maxX; x += spacing) {
        for (float y = minY; y <= maxY; y += spacing) {
            for (float z = minZ; z <= maxZ; z += spacing) {
                float vx, vy, vz;
                vecFunc(x, y, z, vx, vy, vz);
                float len = sqrtf(vx*vx + vy*vy + vz*vz);
                if (len < 1e-6f) { idx++; continue; }
                // Normalise the vector
                vx /= len; vy /= len; vz /= len;
                float scale = 0.3f * spacing;
                float shaftLen = 0.7f * scale;
                float coneLen = 0.3f * scale;
                float sx = x, sy = y, sz = z;
                float ex = x + vx * shaftLen;
                float ey = y + vy * shaftLen;
                float ez = z + vz * shaftLen;
                float ax = x + vx * (shaftLen + coneLen);
                float ay = y + vy * (shaftLen + coneLen);
                float az = z + vz * (shaftLen + coneLen);
                // Color mapping: blue (weak) to red (strong)
                float t = (maxStrength > minStrength) ? (len - minStrength) / (maxStrength - minStrength) : 0.0f;
                float r = t;
                float g = 0.0f;
                float b = 1.0f - t;
                // Arrow shaft (line)
                vertices.push_back(sx); vertices.push_back(sy); vertices.push_back(sz);
                colors.push_back(r); colors.push_back(g); colors.push_back(b);
                vertices.push_back(ex); vertices.push_back(ey); vertices.push_back(ez);
                colors.push_back(r); colors.push_back(g); colors.push_back(b);
                // Arrowhead (3D cone)
                // Build orthonormal basis for cone
                float dirx = vx, diry = vy, dirz = vz;
                float dirLen = sqrtf(dirx*dirx + diry*diry + dirz*dirz);
                dirx /= dirLen; diry /= dirLen; dirz /= dirLen;
                // Find a vector not parallel to dir
                float upx = 0.0f, upy = 1.0f, upz = 0.0f;
                if (fabs(diry) > 0.99f) { upx = 1.0f; upy = 0.0f; upz = 0.0f; }
                // Orthonormal basis
                float basex = upy*dirz - upz*diry;
                float basey = upz*dirx - upx*dirz;
                float basez = upx*diry - upy*dirx;
                float baseLen = sqrtf(basex*basex + basey*basey + basez*basez) + 1e-6f;
                basex /= baseLen; basey /= baseLen; basez /= baseLen;
                float basex2 = diry*basez - dirz*basey;
                float basey2 = dirz*basex - dirx*basez;
                float basez2 = dirx*basey - diry*basex;
                // Cone base radius
                float coneRadius = 0.13f * spacing;
                // Cone base center
                float cx = ex, cy = ey, cz = ez;
                // Cone tip
                float tx = ax, ty = ay, tz = az;
                // Add cone triangles (fan)
                for (int i = 0; i < coneSegments; ++i) {
                    float theta0 = 2.0f * M_PI * i / coneSegments;
                    float theta1 = 2.0f * M_PI * (i+1) / coneSegments;
                    float bx0 = cosf(theta0) * basex + sinf(theta0) * basex2;
                    float by0 = cosf(theta0) * basey + sinf(theta0) * basey2;
                    float bz0 = cosf(theta0) * basez + sinf(theta0) * basez2;
                    float bx1 = cosf(theta1) * basex + sinf(theta1) * basex2;
                    float by1 = cosf(theta1) * basey + sinf(theta1) * basey2;
                    float bz1 = cosf(theta1) * basez + sinf(theta1) * basez2;
                    float px0 = cx + coneRadius * bx0;
                    float py0 = cy + coneRadius * by0;
                    float pz0 = cz + coneRadius * bz0;
                    float px1 = cx + coneRadius * bx1;
                    float py1 = cy + coneRadius * by1;
                    float pz1 = cz + coneRadius * bz1;
                    // Triangle: tip, base0, base1
                    vertices.push_back(tx); vertices.push_back(ty); vertices.push_back(tz);
                    colors.push_back(r); colors.push_back(g); colors.push_back(b);
                    vertices.push_back(px0); vertices.push_back(py0); vertices.push_back(pz0);
                    colors.push_back(r); colors.push_back(g); colors.push_back(b);
                    vertices.push_back(px1); vertices.push_back(py1); vertices.push_back(pz1);
                    colors.push_back(r); colors.push_back(g); colors.push_back(b);
                }
                idx++;
            }
        }
    }
    // Upload to VBO and draw with current shader (with color)
    GLuint VAO, VBO[2];
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glBindVertexArray(VAO);
    // Positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    // Colors
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    // Draw shafts (lines) and cones (triangles)
    int numLines = 0;
    int numTriangles = 0;
    int vertsPerArrow = 2 + 3 * coneSegments;
    int totalArrows = (int)strengths.size();
    numLines = totalArrows * 2;
    numTriangles = totalArrows * coneSegments * 3;
    glDrawArrays(GL_LINES, 0, numLines);
    glDrawArrays(GL_TRIANGLES, numLines, vertices.size()/3 - numLines);
    glBindVertexArray(0);
    glDeleteBuffers(2, VBO);
    glDeleteVertexArrays(1, &VAO);
}
#include "render_utils.hpp"
#include <GL/glew.h>
#include <vector>
#include <cmath>
GLuint createShaderProgram(const char* vShaderSrc, const char* fShaderSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderSrc, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderSrc, NULL);
    glCompileShader(fragmentShader);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void generateGridVertices(std::vector<float>& vertices, int N) {
    vertices.clear();
    // 3D grid: lines in XY, XZ, YZ planes
    float gridSize = 10.0f;
    for (int i = -N; i <= N; ++i) {
        float t = (float)i / N * gridSize;
        // XY plane (Z=0)
        vertices.push_back(-gridSize); vertices.push_back(t); vertices.push_back(0.0f);
        vertices.push_back(gridSize);  vertices.push_back(t); vertices.push_back(0.0f);
        vertices.push_back(t); vertices.push_back(-gridSize); vertices.push_back(0.0f);
        vertices.push_back(t); vertices.push_back(gridSize);  vertices.push_back(0.0f);
        // XZ plane (Y=0)
        vertices.push_back(-gridSize); vertices.push_back(0.0f); vertices.push_back(t);
        vertices.push_back(gridSize);  vertices.push_back(0.0f); vertices.push_back(t);
        vertices.push_back(t); vertices.push_back(0.0f); vertices.push_back(-gridSize);
        vertices.push_back(t); vertices.push_back(0.0f); vertices.push_back(gridSize);
        // YZ plane (X=0)
        vertices.push_back(0.0f); vertices.push_back(-gridSize); vertices.push_back(t);
        vertices.push_back(0.0f); vertices.push_back(gridSize);  vertices.push_back(t);
        vertices.push_back(0.0f); vertices.push_back(t); vertices.push_back(-gridSize);
        vertices.push_back(0.0f); vertices.push_back(t); vertices.push_back(gridSize);
    }
}

void generateAxisVertices(std::vector<float>& vertices) {
    // X axis (red)
    vertices.push_back(-5.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back( 5.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    // Y axis (green)
    vertices.push_back(0.0f); vertices.push_back(-5.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back( 5.0f); vertices.push_back(0.0f);
    // Z axis (blue)
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-5.0f);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back( 5.0f);
}

void setupVAOandVBO(GLuint& VAO, GLuint& VBO, const std::vector<float>& vertices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
