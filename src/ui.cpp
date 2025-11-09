#include "ui.hpp"
#include "physics.hpp"
#include <imgui.h>
#include <cmath>

void drawUI(UIState& state, GLFWwindow* window, PhysicsWorld* world) {
    if (!world) return;
    
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);
    
    // Main control panel
    ImVec2 mainWinSize(450, 600);
    ImVec2 mainWinPos(40, (display_h - mainWinSize.y) * 0.5f);
    ImGui::SetNextWindowPos(mainWinPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(mainWinSize, ImGuiCond_Once);
    
    ImGui::Begin("Physics Simulator Controls", nullptr, ImGuiWindowFlags_NoCollapse);
    
    // === SIMULATION CONTROLS ===
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Gravity", &state.gravity, -2.0f, 2.0f);
        ImGui::SliderFloat("Time Scale", &state.timeScale, 0.01f, 3.0f, "%.2fx");
        ImGui::Checkbox("Paused", &state.paused);
        ImGui::SameLine();
        if (ImGui::Button("Step")) {
            world->step(1.0f / 60.0f);
        }
        
        // NEW: Advanced physics options
        ImGui::Separator();
        ImGui::SliderFloat("Air Drag", &world->airDragCoefficient, 0.0f, 0.1f, "%.4f");
        ImGui::Checkbox("Relativistic Effects", &world->relativisticEffects);
        
        if (ImGui::Button("Clear All")) {
            world->objects.clear();
            world->stats.reset();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Stats")) {
            world->stats.reset();
        }
    }
    
    // === DIAGNOSTICS ===
    if (ImGui::CollapsingHeader("Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Objects: %zu", world->objects.size());
        
        float ke = world->totalKineticEnergy();
        float pe = world->totalPotentialEnergy();
        float total = ke + pe;
        ImGui::Text("Kinetic E: %.3f", ke);
        ImGui::Text("Potential E: %.3f", pe);
        ImGui::Text("Total E: %.3f", total);
        
        float px, py;
        world->totalMomentum(px, py);
        float pMag = std::sqrt(px * px + py * py);
        ImGui::Text("Momentum: %.3f", pMag);
        
        float L = world->totalAngularMomentum();
        ImGui::Text("Angular Mom: %.3f", L);
        
        ImGui::Separator();
        ImGui::Text("Collisions: %zu", world->stats.totalCollisions);
        ImGui::Text("Absorbed: %zu", world->stats.objectsAbsorbed);
        ImGui::Text("Energy Lost: %.3f", world->stats.totalEnergyLost);
    }
    
    // === FORCE FIELDS ===
    if (ImGui::CollapsingHeader("Force Fields")) {
        static int fieldType = 0;
        static float fieldX = 0.0f, fieldY = 0.0f;
        static float fieldStrength = 1.0f, fieldRadius = 0.3f;
        static float fieldAngle = 0.0f;
        
        ImGui::Combo("Type", &fieldType, "Radial\0Vortex\0Directional\0");
        ImGui::InputFloat2("Position", &fieldX);
        ImGui::SliderFloat("Strength", &fieldStrength, -5.0f, 5.0f);
        ImGui::SliderFloat("Radius", &fieldRadius, 0.1f, 1.0f);
        
        if (fieldType == 2) { // Directional
            ImGui::SliderAngle("Angle", &fieldAngle);
        }
        
        if (ImGui::Button("Add Force Field")) {
            ForceField field;
            field.type = static_cast<ForceField::Type>(fieldType);
            field.x = fieldX;
            field.y = fieldY;
            field.strength = fieldStrength;
            field.radius = fieldRadius;
            field.angle = fieldAngle;
            world->addForceField(field);
        }
        
        ImGui::Separator();
        ImGui::Text("Active Fields: %zu", world->forceFields.size());
        for (size_t i = 0; i < world->forceFields.size(); ++i) {
            ImGui::PushID(i);
            ImGui::Checkbox("##active", &world->forceFields[i].active);
            ImGui::SameLine();
            ImGui::Text("Field %zu", i);
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                world->removeForceField(i);
                --i;
            }
            ImGui::PopID();
        }
        
        if (!world->forceFields.empty() && ImGui::Button("Clear All Fields")) {
            world->clearForceFields();
        }
    }
    
    // === PRESET SCENARIOS ===
    if (ImGui::CollapsingHeader("Scenarios")) {
        if (ImGui::Button("Solar System", ImVec2(-1, 0))) {
            world->objects.clear();
            // Sun
            auto sun = ParticleUtils::createStar(0, 0, 20.0f, 5778.0f);
            sun.isStatic = true;
            world->addObject(sun);
            
            // Planets with proper settings
            struct PlanetDef { float r, rad, m, spin; Color3 c; bool gas; };
            std::vector<PlanetDef> planets = {
                {0.13f, 0.018f, 0.3f, 3.0f, {0.7f, 0.7f, 0.7f}, false}, // Mercury
                {0.17f, 0.022f, 0.6f, 2.5f, {0.9f, 0.7f, 0.4f}, false}, // Venus
                {0.22f, 0.024f, 0.7f, 2.0f, {0.2f, 0.5f, 1.0f}, false}, // Earth
                {0.28f, 0.020f, 0.5f, 2.2f, {1.0f, 0.4f, 0.2f}, false}, // Mars
                {0.36f, 0.045f, 2.0f, 1.5f, {0.9f, 0.8f, 0.5f}, true},  // Jupiter
                {0.44f, 0.038f, 1.5f, 1.2f, {0.8f, 0.9f, 0.7f}, true},  // Saturn
            };
            
            for (size_t i = 0; i < planets.size(); ++i) {
                auto& def = planets[i];
                auto p = ParticleUtils::createPlanet(0, 0, def.rad, def.m, def.gas);
                p.orbitTarget = 0;
                p.orbitRadius = def.r;
                p.orbitAngle = float(i) * 0.7f;
                p.color = def.c;
                p.spin = def.spin;
                p.tidallyLocked = (i < 2); // Inner planets tidally locked
                world->addObject(p);
            }
        }
        
        if (ImGui::Button("Binary Stars", ImVec2(-1, 0))) {
            world->objects.clear();
            auto s1 = ParticleUtils::createStar(-0.3f, 0, 6.0f, 6000.0f);
            auto s2 = ParticleUtils::createStar(0.3f, 0, 6.0f, 4500.0f);
            s1.vy = 0.08f; s2.vy = -0.08f;
            world->addObject(s1);
            world->addObject(s2);
        }
        
        if (ImGui::Button("Black Hole + Accretion", ImVec2(-1, 0))) {
            world->objects.clear();
            auto bh = ParticleUtils::createBlackHole(0, 0, 15.0f);
            bh.isStatic = true;
            world->addObject(bh);
            
            world->createAsteroidBelt(0, 0, 0.15f, 0.25f, 30);
        }
        
        if (ImGui::Button("Galaxy Formation", ImVec2(-1, 0))) {
            world->objects.clear();
            world->createGalaxy(0, 0, 3, 15);
        }
        
        if (ImGui::Button("Supernova Demo", ImVec2(-1, 0))) {
            world->objects.clear();
            auto star = ParticleUtils::createStar(0, 0, 12.0f, 8000.0f);
            star.lifetime = 5.0f; // Will go supernova in 5 seconds
            world->addObject(star);
        }
    }
    
    // === OBJECT CREATOR ===
    if (ImGui::CollapsingHeader("Create Objects")) {
        static int objType = 0;
        static float px = 0.0f, py = 0.0f, pvx = 0.0f, pvy = 0.0f;
        static float pradius = 0.03f, pmass = 1.0f;
        static float pspin = 0.0f, ptemp = 273.0f;
        static bool pisStatic = false, ptidalLock = false;
        static float pcolor[3] = {1.0f, 0.0f, 0.0f};
        
        const char* types[] = {"Normal", "Star", "Planet", "Gas Giant", 
                               "BlackHole", "NeutronStar", "Asteroid", "Comet"};
        ImGui::Combo("Type", &objType, types, IM_ARRAYSIZE(types));
        
        ImGui::InputFloat2("Position", &px);
        ImGui::InputFloat2("Velocity", &pvx);
        ImGui::InputFloat("Radius", &pradius, 0.001f, 0.01f, "%.3f");
        ImGui::InputFloat("Mass", &pmass);
        ImGui::InputFloat("Spin", &pspin, 0.1f, 1.0f);
        
        if (objType == 1) { // Star
            ImGui::InputFloat("Temperature (K)", &ptemp);
        }
        
        ImGui::ColorEdit3("Color", pcolor);
        ImGui::Checkbox("Static", &pisStatic);
        ImGui::Checkbox("Tidally Locked", &ptidalLock);
        
        if (ImGui::Button("Add Object", ImVec2(-1, 0))) {
            Particle p;
            p.x = px; p.y = py;
            p.vx = pvx; p.vy = pvy;
            p.radius = pradius;
            p.mass = pmass;
            p.spin = pspin;
            p.isStatic = pisStatic;
            p.tidallyLocked = ptidalLock;
            p.color = {pcolor[0], pcolor[1], pcolor[2]};
            
            switch (objType) {
                case 0: p.type = ObjectType::Normal; break;
                case 1: 
                    p.type = ObjectType::Star; 
                    p.temperature = ptemp;
                    p.emitsLight = true;
                    p.luminosity = pmass * 0.1f;
                    break;
                case 2: p.type = ObjectType::RockyPlanet; break;
                case 3: p.type = ObjectType::GasGiant; break;
                case 4: 
                    p = ParticleUtils::createBlackHole(px, py, pmass);
                    break;
                case 5: 
                    p = ParticleUtils::createNeutronStar(px, py, pmass);
                    break;
                case 6: p.type = ObjectType::Asteroid; break;
                case 7: 
                    p = ParticleUtils::createComet(px, py, pvx, pvy);
                    break;
            }
            
            world->addObject(p);
        }
    }
    
    // === VISUAL OPTIONS ===
    if (ImGui::CollapsingHeader("Visuals")) {
        ImGui::Checkbox("Show Trails", &state.showTrails);
        ImGui::Checkbox("Show Labels", &state.showLabels);
        ImGui::Checkbox("Show Field", &state.showField);
        ImGui::Checkbox("Show Axes", &state.showAxes);
    }
    
    ImGui::End();
}