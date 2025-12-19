#pragma once
#include <vector>
#include <cmath>
#include <memory>

// Color struct for rendering
struct Color3 {
    float r, g, b;
    
    // Helper for color interpolation
    static Color3 lerp(const Color3& a, const Color3& b, float t) {
        return {
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t
        };
    }
};

// Particle/object type
enum class ObjectType {
    Normal,
    Merged,
    BlackHole,
    Star,
    Planet,
    Asteroid,
    Comet,        // NEW: Comet with trail
    NeutronStar,  // NEW: Dense, rapidly spinning
    WhiteDwarf,   // NEW: Remnant star
    GasGiant,     // NEW: Large planet with rings
    RockyPlanet   // NEW: Small terrestrial planet
};

// Trail system for visual effects
struct Trail {
    std::vector<float> positions; // x1,y1,x2,y2,...
    size_t maxPoints = 100;
    float fadeRate = 0.98f;
    
    void addPoint(float x, float y);
    void update();
    void clear();
};

// Particle/physics object
struct Particle {
    float x, y;
    float vx, vy;
    float radius;
    float mass;
    float charge = 0.0f;
    bool isStatic = false;
    ObjectType type = ObjectType::Normal;
    Color3 color = {1.0f, 0.0f, 0.0f};
    std::vector<Particle> components;
    
    // Celestial properties
    float eventHorizon = 0.0f;
    float luminosity = 0.0f;
    float absorption = 0.0f;
    float orbitRadius = 0.0f;
    float orbitAngle = 0.0f;
    int orbitTarget = -1;
    
    // Visual effects
    float spin = 0.0f;
    float spinAngle = 0.0f;
    std::shared_ptr<Trail> trail; // Optional trail rendering
    
    // NEW: Enhanced physics properties
    float temperature = 273.0f;  // Kelvin
    float density = 1.0f;        // Relative density
    bool emitsLight = false;     // Light emission flag
    float magneticField = 0.0f;  // Magnetic field strength
    
    // NEW: Lifetime and decay
    float lifetime = -1.0f;      // -1 = infinite, else decreases
    float age = 0.0f;
    bool decaying = false;
    
    // NEW: Collision elasticity per-object
    float restitution = 0.95f;
    
    // NEW: Tidal locking for moons
    bool tidallyLocked = false;
    
    // Constructor
    Particle() = default;
    
    // Helper methods
    float kineticEnergy() const {
        if (isStatic) return 0.0f;
        return 0.5f * mass * (vx * vx + vy * vy);
    }
    
    float distanceTo(const Particle& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    void updateAge(float dt) {
        if (lifetime > 0) {
            age += dt;
            if (age >= lifetime) decaying = true;
        }
    }
    
    // Schwarzschild radius for black holes
    float schwarzschildRadius() const {
        return 2.0f * mass * 0.001f; // Scaled for simulation
    }
};

// Utility functions for particles
namespace ParticleUtils {
    Color3 getColor(const Particle& p, float minMass = 1.0f, float maxMass = 10.0f);
    float getSize(const Particle& p);
    void computeMassRange(const std::vector<Particle>& particles, float& minMass, float& maxMass);
    
    // NEW: Temperature-based color for stars
    Color3 getTemperatureColor(float temp);
    
    // NEW: Create preset objects
    Particle createStar(float x, float y, float mass, float temp = 5778.0f);
    Particle createPlanet(float x, float y, float radius, float mass, bool gasGiant = false);
    Particle createBlackHole(float x, float y, float mass);
    Particle createComet(float x, float y, float vx, float vy);
    Particle createNeutronStar(float x, float y, float mass);
}