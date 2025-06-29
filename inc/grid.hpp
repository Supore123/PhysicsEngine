#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class PhysicsWorld;

// Handles drawing the gravity field grid and axes
class GridRenderer {
public:
    GridRenderer(int fieldN = 20, float arrowScale = 0.07f, float arrowAlpha = 0.25f);
    void drawAxes(GLuint axisProgram, GLuint axisVAO, int axisVertexCount);
    void drawField(const PhysicsWorld& world, GLuint axisProgram, GLint colorLoc);
private:
    int fieldN;
    float arrowScale;
    float arrowAlpha;
};

