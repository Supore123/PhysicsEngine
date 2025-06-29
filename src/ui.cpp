#include "ui.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static bool showDemoWindow = true; // Start enabled for debug

void drawUI(UIState& state, GLFWwindow* window) {
    // Place Simulation Controls at left side, vertically centered, allow moving/resizing
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);
    ImVec2 simWinSize(400, 400);
    ImVec2 simWinPos(40, (display_h - simWinSize.y) * 0.5f);
    ImGui::SetNextWindowPos(simWinPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(simWinSize, ImGuiCond_Once);
    ImGui::Begin("Simulation Controls", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SliderFloat("Gravity", &state.gravity, -2.0f, 2.0f);
    ImGui::SliderFloat("Restitution", &state.restitution, 0.0f, 1.0f);
    ImGui::SliderFloat("Friction", &state.friction, 0.0f, 0.5f);
    ImGui::SliderFloat("Time Scale", &state.timeScale, 0.01f, 2.0f, "%.2fx");
    ImGui::Checkbox("Show Trails", &state.showTrails);
    ImGui::Checkbox("Show Labels", &state.showLabels);
    ImGui::Checkbox("Show Field", &state.showField);
    ImGui::Checkbox("Show Axes", &state.showAxes);
    ImGui::Checkbox("Paused", &state.paused);
    ImGui::Separator();
    ImGui::End();
}
