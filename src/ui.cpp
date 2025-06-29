#include "ui.hpp"
#include "physics.hpp"
#include <imgui.h>
#include <cmath>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static bool showDemoWindow = true; // Start enabled for debug

// Now takes PhysicsWorld* for diagnostics display
void drawUI(UIState& state, GLFWwindow* window, PhysicsWorld* world) {
    // --- Example Scenarios ---
    ImGui::Begin("Example Scenarios");
    if (ImGui::Button("Planet Orbiting Star") && world) {
        world->objects.clear();
        // Add a star at the center
        Particle star;
        star.x = 0.0f; star.y = 0.0f;
        star.vx = 0.0f; star.vy = 0.0f;
        star.radius = 0.07f;
        star.mass = 10.0f;
        star.isStatic = true;
        star.type = ObjectType::Star;
        star.luminosity = 1.0f;
        star.color = {1.0f, 0.9f, 0.2f};
        world->addObject(star);
        // Add a planet in orbit
        Particle planet;
        planet.orbitTarget = 0; // index of star
        planet.orbitRadius = 0.35f;
        planet.orbitAngle = 0.0f;
        planet.x = star.x + planet.orbitRadius;
        planet.y = star.y;
        planet.radius = 0.03f;
        planet.mass = 1.0f;
        planet.isStatic = false;
        planet.type = ObjectType::Planet;
        planet.color = {0.2f, 0.5f, 1.0f};
        float v = sqrt(0.5f * star.mass / std::max(planet.orbitRadius, 1e-4f));
        planet.vx = 0.0f;
        planet.vy = v;
        world->addObject(planet);
    }

    if (ImGui::Button("Binary Star System") && world) {
        world->objects.clear();
        // Two stars orbiting each other
        Particle star1, star2;
        star1.x = -0.15f; star1.y = 0.0f;
        star2.x =  0.15f; star2.y = 0.0f;
        star1.vx = 0.0f; star1.vy = 0.25f;
        star2.vx = 0.0f; star2.vy = -0.25f;
        star1.radius = star2.radius = 0.06f;
        star1.mass = star2.mass = 6.0f;
        star1.isStatic = star2.isStatic = false;
        star1.type = star2.type = ObjectType::Star;
        star1.luminosity = star2.luminosity = 1.0f;
        star1.color = {1.0f, 0.7f, 0.2f};
        star2.color = {1.0f, 0.3f, 0.7f};
        world->addObject(star1);
        world->addObject(star2);
        // Add a planet orbiting both
        Particle planet;
        planet.orbitTarget = 0; // orbit first star for simplicity
        planet.orbitRadius = 0.45f;
        planet.orbitAngle = 0.0f;
        planet.x = star1.x + planet.orbitRadius;
        planet.y = star1.y;
        planet.radius = 0.025f;
        planet.mass = 0.8f;
        planet.isStatic = false;
        planet.type = ObjectType::Planet;
        planet.color = {0.2f, 1.0f, 0.7f};
        float v = sqrt(0.5f * (star1.mass + star2.mass) / std::max(planet.orbitRadius, 1e-4f));
        planet.vx = 0.0f;
        planet.vy = v;
        world->addObject(planet);
    }

    if (ImGui::Button("Black Hole with Accretion Disk") && world) {
        world->objects.clear();
        // Add a black hole at the center
        Particle bh;
        bh.x = 0.0f; bh.y = 0.0f;
        bh.vx = 0.0f; bh.vy = 0.0f;
        bh.radius = 0.06f;
        bh.mass = 15.0f;
        bh.isStatic = true;
        bh.type = ObjectType::BlackHole;
        bh.eventHorizon = 0.09f;
        bh.absorption = 1.0f;
        bh.color = {0.1f, 0.1f, 0.1f};
        world->addObject(bh);
        // Add accretion disk (many small asteroids in orbit)
        for (int i = 0; i < 18; ++i) {
            float angle = i * (2.0f * 3.1415926f / 18);
            Particle ast;
            ast.orbitTarget = 0;
            ast.orbitRadius = 0.18f + 0.02f * (i % 3);
            ast.orbitAngle = angle;
            ast.x = bh.x + ast.orbitRadius * cos(angle);
            ast.y = bh.y + ast.orbitRadius * sin(angle);
            ast.radius = 0.012f;
            ast.mass = 0.15f;
            ast.isStatic = false;
            ast.type = ObjectType::Asteroid;
            ast.color = {0.7f, 0.6f, 0.4f};
            float v = sqrt(0.5f * bh.mass / std::max(ast.orbitRadius, 1e-4f));
            ast.vx = -v * sin(angle);
            ast.vy =  v * cos(angle);
            world->addObject(ast);
        }
    }
    ImGui::End();
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
    // --- Diagnostics: Energy and Momentum ---
    if (world) {
        float ke = world->totalKineticEnergy();
        float px = 0.0f, py = 0.0f;
        world->totalMomentum(px, py);
        ImGui::Text("Kinetic Energy: %.3f", ke);
        ImGui::Text("Momentum: (%.3f, %.3f)", px, py);
    }
    ImGui::Separator();
    // --- ImGui controls for creating new particles ---
    static int selectedType = 0;
    static float px = 0.0f, py = 0.0f, pvx = 0.0f, pvy = 0.0f;
    static float pradius = 0.03f, pmass = 1.0f, pcharge = 0.0f;
    static bool pisStatic = false;
    static float pcolor[3] = {1.0f, 0.0f, 0.0f};
    static float eventHorizon = 0.1f, luminosity = 0.0f, absorption = 1.0f;
    static float orbitRadius = 0.2f, orbitAngle = 0.0f;
    static int orbitTarget = -1;

    ImGui::Text("Create New Object");
    const char* typeNames[] = {"Normal", "BlackHole", "Star", "Planet", "Asteroid"};
    ImGui::Combo("Type", &selectedType, typeNames, IM_ARRAYSIZE(typeNames));
    ImGui::InputFloat2("Position (x, y)", &px);
    ImGui::InputFloat2("Velocity (vx, vy)", &pvx);
    ImGui::InputFloat("Radius", &pradius);
    ImGui::InputFloat("Mass", &pmass);
    ImGui::InputFloat("Charge", &pcharge);
    ImGui::Checkbox("Static", &pisStatic);
    ImGui::ColorEdit3("Color", pcolor);
    if (selectedType == 1) { // BlackHole
        ImGui::InputFloat("Event Horizon", &eventHorizon);
        ImGui::InputFloat("Absorption", &absorption);
    }
    if (selectedType == 2) { // Star
        ImGui::InputFloat("Luminosity", &luminosity);
    }
    if (selectedType == 3) { // Planet
        ImGui::InputFloat("Orbit Radius", &orbitRadius);
        ImGui::InputFloat("Orbit Angle", &orbitAngle);
        ImGui::InputInt("Orbit Target (index)", &orbitTarget);
    }
    if (ImGui::Button("Add Object") && world) {
        Particle p;
        p.x = px; p.y = py;
        p.vx = pvx; p.vy = pvy;
        p.radius = pradius;
        p.mass = pmass;
        p.charge = pcharge;
        p.isStatic = pisStatic;
        p.color = {pcolor[0], pcolor[1], pcolor[2]};
        switch (selectedType) {
            case 0: p.type = ObjectType::Normal; break;
            case 1: p.type = ObjectType::BlackHole; p.eventHorizon = eventHorizon; p.absorption = absorption; break;
            case 2: p.type = ObjectType::Star; p.luminosity = luminosity; break;
            case 3: p.type = ObjectType::Planet; p.orbitRadius = orbitRadius; p.orbitAngle = orbitAngle; p.orbitTarget = orbitTarget; break;
            case 4: p.type = ObjectType::Asteroid; break;
        }
        world->addObject(p);
    }
    ImGui::End();
}
