#pragma once
#include <vector>

// Color struct for rendering
struct Color3 {
    float r, g, b;
};

// Particle/object type
/**
 * ObjectType:
 *  - Normal: Standard particle
 *  - Merged: Result of a merge
 *  - BlackHole: Extremely massive, absorbs others within eventHorizon
 *  - Star: Massive, can be static or moving, may emit light
 *  - Planet: For orbit simulation, can orbit a Star or BlackHole
 *  - Asteroid: Small, can be destroyed/absorbed
 */
enum class ObjectType {
    Normal,
    Merged,
    BlackHole,   // Extremely massive, absorbs others
    Star,        // Massive, can be static or moving
    Planet,      // For orbit simulation
    Asteroid     // Small, can be destroyed/absorbed
};

// Particle/physics object
/**
 * Particle:
 *  - x, y: Position
 *  - vx, vy: Velocity
 *  - radius: Visual/physical radius
 *  - mass: Mass (affects gravity, inertia)
 *  - charge: For future electric/magnetic effects
 *  - isStatic: If true, does not move
 *  - type: See ObjectType
 *  - color: For rendering
 *  - components: Used if type == Merged (originals)
 *  - eventHorizon: For BlackHole (radius of no return)
 *  - luminosity: For Star (brightness)
 *  - absorption: For BlackHole (absorption strength)
 *  - orbitRadius, orbitAngle, orbitTarget: For Planet orbits
 */
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float radius;
    float mass;
    float charge = 0.0f;
    bool isStatic = false;
    ObjectType type = ObjectType::Normal;
    Color3 color = {1.0f, 0.0f, 0.0f};
    std::vector<Particle> components; // Only used if type == Merged
    // For celestial/black hole types:
    float eventHorizon = 0.0f; // For BlackHole: radius of no return
    float luminosity = 0.0f;   // For Star: brightness
    float absorption = 0.0f;   // For BlackHole: how strongly it absorbs
    float orbitRadius = 0.0f;  // For Planet: for scenario setup
    float orbitAngle = 0.0f;   // For Planet: for scenario setup
    int orbitTarget = -1;      // Index of object being orbited (if any)
    // Visual rotation (3D)
    float spin = 0.0f;         // Angular speed (radians per step)
    float spinAngle = 0.0f;    // Current orientation (radians)
    float spinAxisX = 0.0f;    // 3D spin axis (normalized)
    float spinAxisY = 0.0f;
    float spinAxisZ = 1.0f;
    // You can add more as needed
};

// Utility functions for particles
namespace ParticleUtils {
    Color3 getColor(const Particle& p, float minMass = 1.0f, float maxMass = 10.0f);
    float getSize(const Particle& p);
    void computeMassRange(const std::vector<Particle>& particles, float& minMass, float& maxMass);
}
