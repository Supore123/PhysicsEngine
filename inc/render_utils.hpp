#pragma once
#include <vector>
#include <GL/glew.h>

GLuint createShaderProgram(const char* vShaderSrc, const char* fShaderSrc);
void generateGridVertices(std::vector<float>& vertices, int N);
void generateAxisVertices(std::vector<float>& vertices);
void setupVAOandVBO(GLuint& VAO, GLuint& VBO, const std::vector<float>& vertices);
