#pragma once
#include <GLFW/glfw3.h>

struct UIState {
    float gravity = 0.0f;
    float restitution = 0.95f;
    float friction = 0.08f;
    float timeScale = 1.0f;
    bool showTrails = true;
    bool showLabels = true;
    bool showField = true;
    bool showAxes = true;
    bool paused = false;
};

void drawUI(UIState& state, GLFWwindow* window);
