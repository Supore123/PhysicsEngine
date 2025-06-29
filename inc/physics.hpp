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

    void addObject(const PhysicsObject& obj);
    void step(float dt);
    void handleCollisions();
    void handleWalls();
    void applyGravityForces();
};
