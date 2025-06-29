#include "render_utils.hpp"
#include <GL/glew.h>
#include <vector>

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
}

void generateAxisVertices(std::vector<float>& vertices) {
    vertices.push_back(-1.0f); vertices.push_back(0.0f);
    vertices.push_back( 1.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(-1.0f);
    vertices.push_back(0.0f); vertices.push_back( 1.0f);
}

void setupVAOandVBO(GLuint& VAO, GLuint& VBO, const std::vector<float>& vertices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
