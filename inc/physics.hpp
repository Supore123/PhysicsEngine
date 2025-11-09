#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include "particle.hpp"

using PhysicsObject = Particle;

// NEW: Force field system for custom physics
struct ForceField {
    enum Type { RADIAL, DIRECTIONAL, VORTEX, CUSTOM };
    Type type;
    float x, y;          // Center position
    float strength;
    float radius;        // Influence radius
    float angle = 0.0f;  // For directional forces
    std::function<void(PhysicsObject&, float, float)> customForce;
    bool active = true;
};

class PhysicsWorld {
public:
    std::vector<PhysicsObject> objects;
    float gravity = 0.0f;
    float left = -1.0f;
    float right = 1.0f;
    float bottom = -1.0f;
    float top = 1.0f;

    // Spatial Partitioning
    int gridRows = 20;
    int gridCols = 20;
    float cellWidth = 0.1f;
    float cellHeight = 0.1f;
    std::vector<std::vector<std::vector<size_t>>> gridCells;

    // NEW: Force fields system
    std::vector<ForceField> forceFields;
    
    // NEW: Global physics settings
    float airDragCoefficient = 0.0f;  // Atmospheric drag
    bool relativisticEffects = false;  // Enable relativistic corrections near black holes
    float timeWarpFactor = 1.0f;       // Time dilation near massive objects
    
    // NEW: Collision statistics
    struct Stats {
        size_t totalCollisions = 0;
        size_t objectsAbsorbed = 0;
        float totalEnergyLost = 0.0f;
        void reset() { totalCollisions = 0; objectsAbsorbed = 0; totalEnergyLost = 0.0f; }
    } stats;

    void updateSpatialGrid();
    void addObject(const PhysicsObject& obj);
    void step(float dt);
    void handleCollisions();
    void handleWalls();
    void applyGravityForces();
    
    // NEW: Enhanced force methods
    void applyForceFields();
    void applyAirDrag(float dt);
    void applyTidalForces();
    void updateTemperatures(float dt);
    
    // Diagnostics
    float totalKineticEnergy() const;
    void totalMomentum(float& px, float& py) const;
    float totalPotentialEnergy() const;  // NEW
    float totalAngularMomentum() const;  // NEW
    
    // NEW: Advanced features
    void handleSupernova(size_t starIndex);
    void formBinarySystem(size_t idx1, size_t idx2);
    void createDebrisField(float x, float y, int count, float speed);
    
    // NEW: Force field management
    void addForceField(const ForceField& field);
    void removeForceField(size_t index);
    void clearForceFields();
    
    // NEW: Scenario helpers
    void createGalaxy(float centerX, float centerY, int armCount, int starsPerArm);
    void createAsteroidBelt(float centerX, float centerY, float innerR, float outerR, int count);
    
private:
    // NEW: Helper for relativistic time dilation
    float getTimeDilation(const PhysicsObject& obj) const;
    
    // NEW: Helper for tidal radius
    float getTidalRadius(const PhysicsObject& obj1, const PhysicsObject& obj2) const;
};

// NEW: Inline implementations for performance
inline float PhysicsWorld::getTimeDilation(const PhysicsObject& obj) const {
    if (!relativisticEffects) return 1.0f;
    
    // Find nearest massive object
    float minFactor = 1.0f;
    for (const auto& other : objects) {
        if (other.type == ObjectType::BlackHole && &other != &obj) {
            float dist = obj.distanceTo(other);
            float rs = other.schwarzschildRadius();
            if (dist > rs) {
                float factor = std::sqrt(1.0f - rs / dist);
                if (factor < minFactor) minFactor = factor;
            }
        }
    }
    return minFactor;
}

inline float PhysicsWorld::getTidalRadius(const PhysicsObject& obj1, const PhysicsObject& obj2) const {
    // Roche limit approximation
    return 2.44f * obj2.radius * std::pow(obj2.density / obj1.density, 1.0f/3.0f);
}