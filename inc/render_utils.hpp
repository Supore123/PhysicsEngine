// Draw a 3D vector field (arrows) across the entire axis-aligned box
void drawVectorField3D(float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float spacing, void(*vecFunc)(float,float,float,float&,float&,float&));
#pragma once
#include <vector>
#include <GL/glew.h>

GLuint createShaderProgram(const char* vShaderSrc, const char* fShaderSrc);
void generateGridVertices(std::vector<float>& vertices, int N);
void generateAxisVertices(std::vector<float>& vertices);
void setupVAOandVBO(GLuint& VAO, GLuint& VBO, const std::vector<float>& vertices);
