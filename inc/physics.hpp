
#pragma once
#include <vector>
#include "particle.hpp"
using PhysicsObject = Particle;

class PhysicsWorld {
public:
    std::vector<PhysicsObject> objects;
    float gravity = 0.0f;
    // Boundaries (walls/floor/ceiling) in normalized device coordinates
    float left = -1.0f;
    float right = 1.0f;
    float bottom = -1.0f;
    float top = 1.0f;

    // --- Spatial Partitioning (Uniform Grid) ---
    int gridRows = 20;
    int gridCols = 20;
    float cellWidth = 0.1f;
    float cellHeight = 0.1f;
    // Each cell contains indices of objects in that cell
    std::vector<std::vector<std::vector<size_t>>> gridCells;

    // Call before collision detection each step
    void updateSpatialGrid();

    void addObject(const PhysicsObject& obj);
    void step(float dt);
    void handleCollisions();
    void handleWalls();
    void applyGravityForces();
    // Diagnostics: Energy and Momentum
    float totalKineticEnergy() const;
    void totalMomentum(float& px, float& py) const;
};
