#include "ui.hpp"
#include "physics.hpp"
#include <imgui.h>
#include <cmath>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static bool showDemoWindow = true; // Start enabled for debug

// Now takes PhysicsWorld* for diagnostics display
void drawUI(UIState& state, GLFWwindow* window, PhysicsWorld* world) {
    if (ImGui::Button("Solar System Example") && world) {
        world->objects.clear();
        // Add a central star (the Sun)
        Particle sun;
        sun.x = 0.0f; sun.y = 0.0f; sun.z = 0.0f;
        sun.vx = 0.0f; sun.vy = 0.0f; sun.vz = 0.0f;
        sun.radius = 0.08f;
        sun.mass = 20.0f;
        sun.isStatic = true;
        sun.type = ObjectType::Star;
        sun.luminosity = 1.5f;
        sun.color = {1.0f, 0.9f, 0.2f};
        sun.spin = 1.0f;
        sun.spinAxisX = 0.0f; sun.spinAxisY = 0.0f; sun.spinAxisZ = 1.0f;
        world->addObject(sun);

        // Define planet parameters: {orbitRadius, radius, mass, color, spin, orbitInclination}
        struct PlanetDef { float orbitRadius, radius, mass, spin, inclination; Color3 color; };
        PlanetDef planets[] = {
            {0.13f, 0.018f, 0.3f, 3.0f, 0.1f, {0.7f, 0.7f, 0.7f}}, // Mercury
            {0.17f, 0.022f, 0.6f, 2.5f, 0.2f, {0.9f, 0.7f, 0.4f}}, // Venus
            {0.22f, 0.024f, 0.7f, 2.0f, 0.0f, {0.2f, 0.5f, 1.0f}}, // Earth
            {0.28f, 0.020f, 0.5f, 2.2f, 0.05f, {1.0f, 0.4f, 0.2f}}, // Mars
            {0.36f, 0.045f, 2.0f, 1.5f, 0.3f, {0.9f, 0.8f, 0.5f}}, // Jupiter
            {0.44f, 0.038f, 1.5f, 1.2f, 0.25f, {0.8f, 0.9f, 0.7f}}, // Saturn
            {0.52f, 0.030f, 1.0f, 1.0f, 0.15f, {0.5f, 0.8f, 1.0f}}, // Uranus
            {0.60f, 0.028f, 0.8f, 0.8f, 0.12f, {0.4f, 0.7f, 1.0f}}  // Neptune
        };
        for (size_t i = 0; i < sizeof(planets)/sizeof(planets[0]); ++i) {
            const PlanetDef& def = planets[i];
            Particle planet;
            planet.orbitTarget = 0; // Sun
            planet.orbitRadius = def.orbitRadius;
            planet.orbitAngle = float(i) * 0.7f; // Staggered starting positions
            float inclination = def.inclination;
            planet.x = sun.x + planet.orbitRadius * cos(planet.orbitAngle);
            planet.y = sun.y + planet.orbitRadius * sin(planet.orbitAngle) * cos(inclination);
            planet.z = sun.z + planet.orbitRadius * sin(planet.orbitAngle) * sin(inclination);
            planet.radius = def.radius;
            planet.mass = def.mass;
            planet.isStatic = false;
            planet.type = ObjectType::Planet;
            planet.color = def.color;
            planet.spin = def.spin;
            planet.spinAxisX = 0.0f; planet.spinAxisY = 0.0f; planet.spinAxisZ = 1.0f;
            float v = sqrtf(0.5f * sun.mass / ((planet.orbitRadius > 1e-4f) ? planet.orbitRadius : 1e-4f));
            // 3D velocity: perpendicular to radius vector in orbital plane
            planet.vx = -v * sin(planet.orbitAngle);
            planet.vy =  v * cos(planet.orbitAngle) * cos(inclination);
            planet.vz =  v * cos(planet.orbitAngle) * sin(inclination);
            world->addObject(planet);
        }
    }

    // --- Example Scenarios ---
    ImGui::Begin("Example Scenarios");
    if (ImGui::Button("Planet Orbiting Star") && world) {
    world->objects.clear();
    // Add a spinning star at the center (3D)
    Particle star;
    star.x = 0.0f; star.y = 0.0f; star.z = 0.0f;
    star.vx = 0.0f; star.vy = 0.0f; star.vz = 0.0f;
    star.radius = 0.07f;
    star.mass = 10.0f;
    star.isStatic = true;
    star.type = ObjectType::Star;
    star.luminosity = 1.0f;
    star.color = {1.0f, 0.9f, 0.2f};
    star.spin = 1.2f; // Add visible spin
    star.spinAxisX = 0.0f; star.spinAxisY = 1.0f; star.spinAxisZ = 0.0f;
    world->addObject(star);
    // Add a planet in 3D orbit with spin and inclination
    Particle planet;
    planet.orbitTarget = 0; // index of star
    planet.orbitRadius = 0.35f;
    planet.orbitAngle = 0.0f;
    float inclination = 0.4f; // radians, for 3D tilt
    planet.x = star.x + planet.orbitRadius * cos(planet.orbitAngle);
    planet.y = star.y + planet.orbitRadius * sin(planet.orbitAngle) * cos(inclination);
    planet.z = star.z + planet.orbitRadius * sin(planet.orbitAngle) * sin(inclination);
    planet.radius = 0.03f;
    planet.mass = 1.0f;
    planet.isStatic = false;
    planet.type = ObjectType::Planet;
    planet.color = {0.2f, 0.5f, 1.0f};
    planet.spin = 2.5f; // Add visible spin
    planet.spinAxisX = 0.0f; planet.spinAxisY = 1.0f; planet.spinAxisZ = 0.0f;
    float v = sqrt(0.5f * star.mass / std::max(planet.orbitRadius, 1e-4f));
    // 3D velocity: perpendicular to radius vector in orbital plane
    planet.vx = -v * sin(planet.orbitAngle);
    planet.vy =  v * cos(planet.orbitAngle) * cos(inclination);
    planet.vz =  v * cos(planet.orbitAngle) * sin(inclination);
    world->addObject(planet);

    // Add a moon orbiting the planet (properly in the planet's 3D reference frame)
    Particle moon;
    moon.orbitTarget = 1; // index of planet
    moon.orbitRadius = 0.08f;
    moon.orbitAngle = 0.0f;
    float moonIncl = 0.7f;
    // Place moon at correct position relative to planet
    moon.x = planet.x + moon.orbitRadius * cos(moon.orbitAngle);
    moon.y = planet.y + moon.orbitRadius * sin(moon.orbitAngle) * cos(moonIncl);
    moon.z = planet.z + moon.orbitRadius * sin(moon.orbitAngle) * sin(moonIncl);
    moon.radius = 0.012f;
    moon.mass = 0.1f;
    moon.isStatic = false;
    moon.type = ObjectType::Asteroid; // Or define ObjectType::Moon if you wish
    moon.color = {0.8f, 0.8f, 0.8f};
    moon.spin = 1.0f;
    moon.spinAxisX = 0.0f; moon.spinAxisY = 1.0f; moon.spinAxisZ = 0.0f;
    // Calculate moon's velocity: planet's velocity + orbital velocity around planet (3D)
    float vm = sqrt(0.5f * planet.mass / std::max(moon.orbitRadius, 1e-4f));
    moon.vx = planet.vx - vm * sin(moon.orbitAngle);
    moon.vy = planet.vy + vm * cos(moon.orbitAngle) * cos(moonIncl);
    moon.vz = planet.vz + vm * cos(moon.orbitAngle) * sin(moonIncl);
    world->addObject(moon);
}

    if (ImGui::Button("Binary Star System") && world) {
        world->objects.clear();
        // Two spinning stars
        float m1 = 6.0f, m2 = 6.0f;
        float d = 0.4f; // separation
        float barycenter_x = 0.0f;
        float x1 = barycenter_x - d * m2 / (m1 + m2);
        float x2 = barycenter_x + d * m1 / (m1 + m2);
        float y1 = 0.0f, y2 = 0.0f;
        Particle star1, star2;
        star1.x = x1; star1.y = y1;
        star2.x = x2; star2.y = y2;
        star1.vx = 0.0f; star1.vy = 0.0f;
        star2.vx = 0.0f; star2.vy = 0.0f;
        star1.radius = star2.radius = 0.06f;
        star1.mass = m1; star2.mass = m2;
        star1.isStatic = star2.isStatic = true;
        star1.type = star2.type = ObjectType::Star;
        star1.luminosity = star2.luminosity = 1.0f;
        star1.color = {1.0f, 0.7f, 0.2f};
        star2.color = {1.0f, 0.3f, 0.7f};
        star1.spin = 1.5f;
        star2.spin = -1.5f;
        world->addObject(star1);
        world->addObject(star2);
        // Add a planet stable between the two stars (L1 point) with spin
        Particle planet;
        planet.x = barycenter_x; planet.y = 0.0f;
        planet.vx = 0.0f; planet.vy = 0.0f;
        planet.radius = 0.025f;
        planet.mass = 0.8f;
        planet.isStatic = false;
        planet.type = ObjectType::Planet;
        planet.color = {0.2f, 1.0f, 0.7f};
        planet.spin = 2.0f;
        world->addObject(planet);
    }

    if (ImGui::Button("Black Hole with Accretion Disk") && world) {
        world->objects.clear();
        // Add a spinning black hole at the center
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
        bh.spin = 2.0f;
        world->addObject(bh);
        // Add accretion disk (many small asteroids in orbit, each with spin)
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
            ast.spin = 3.0f * ((i % 2 == 0) ? 1.0f : -1.0f); // Alternate spin direction
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
    ImGui::Checkbox("Show Field (XY plane)", &state.showField);
    ImGui::Checkbox("Show 3D Vector Field (all axes)", &state.showField3D);
    ImGui::Checkbox("Show Axes", &state.showAxes);
    ImGui::Checkbox("Paused", &state.paused);
    ImGui::Separator();
    // --- Diagnostics: Energy and Momentum ---
    if (world) {
        float ke = world->totalKineticEnergy();
        float px = 0.0f, py = 0.0f, pz = 0.0f;
        world->totalMomentum(px, py, pz);
        ImGui::Text("Kinetic Energy: %.3f", ke);
        ImGui::Text("Momentum: (%.3f, %.3f, %.3f)", px, py, pz);
    }
    ImGui::Separator();
    // --- ImGui controls for creating new particles ---
    static int selectedType = 0;
    static float px = 0.0f, py = 0.0f, pz = 0.0f;
    static float pvx = 0.0f, pvy = 0.0f, pvz = 0.0f;
    static float pradius = 0.03f, pmass = 1.0f, pcharge = 0.0f;
    static float pspin = 0.0f;
    static float pspinAxis[3] = {0.0f, 0.0f, 1.0f};
    static bool pisStatic = false;
    static float pcolor[3] = {1.0f, 0.0f, 0.0f};
    static float eventHorizon = 0.1f, luminosity = 0.0f, absorption = 1.0f;
    static float orbitRadius = 0.2f, orbitAngle = 0.0f;
    static int orbitTarget = -1;

    ImGui::Text("Create New Object");
    const char* typeNames[] = {"Normal", "BlackHole", "Star", "Planet", "Asteroid"};
    ImGui::Combo("Type", &selectedType, typeNames, IM_ARRAYSIZE(typeNames));
    ImGui::InputFloat3("Position (x, y, z)", &px);
    ImGui::InputFloat3("Velocity (vx, vy, vz)", &pvx);
    ImGui::InputFloat("Radius", &pradius);
    ImGui::InputFloat("Mass", &pmass);
    ImGui::InputFloat("Charge", &pcharge);
    ImGui::Checkbox("Static", &pisStatic);
    ImGui::ColorEdit3("Color", pcolor);
    ImGui::InputFloat("Spin (rad/s)", &pspin, 0.01f, 0.1f, "%.3f");
    ImGui::InputFloat3("Spin Axis (x, y, z)", pspinAxis);
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
        p.x = px; p.y = py; p.z = pz;
        p.vx = pvx; p.vy = pvy; p.vz = pvz;
        p.radius = pradius;
        p.mass = pmass;
        p.charge = pcharge;
        p.isStatic = pisStatic;
        p.color = {pcolor[0], pcolor[1], pcolor[2]};
        p.spin = pspin;
        p.spinAxisX = pspinAxis[0];
        p.spinAxisY = pspinAxis[1];
        p.spinAxisZ = pspinAxis[2];
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
