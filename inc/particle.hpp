#pragma once
#include <vector>

// Color struct for rendering
struct Color3 {
    float r, g, b;
};

// Particle/object type
enum class ObjectType {
    Normal,
    Merged
};

// Particle/physics object
struct Particle {
    float x, y;
    float vx, vy;
    float radius;
    float mass;
    float charge = 0.0f;
    bool isStatic;
    ObjectType type = ObjectType::Normal;
    Color3 color = {1.0f, 0.0f, 0.0f};
    std::vector<Particle> components; // Only used if type == Merged
};

// Utility functions for particles
namespace ParticleUtils {
    Color3 getColor(const Particle& p, float minMass = 1.0f, float maxMass = 10.0f);
    float getSize(const Particle& p);
    void computeMassRange(const std::vector<Particle>& particles, float& minMass, float& maxMass);
}
