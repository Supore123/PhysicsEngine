#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "physics.hpp"
#include "grid.hpp"

// Add camera parameter
#include "camera.hpp"
#include "ui.hpp"
void renderLoop(GLFWwindow* window, GLuint gridProgram, GLuint gridVAO, int gridVertexCount, GLuint axisProgram, GLuint axisVAO, int axisVertexCount, GridRenderer& gridRenderer, PhysicsWorld& world, GLuint pointProgram, const Camera3D& camera, UIState& uiState);
