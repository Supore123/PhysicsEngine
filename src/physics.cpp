#include "physics.hpp"
#include <cmath>
#include <algorithm>

void PhysicsWorld::addObject(const PhysicsObject& obj) {
    objects.push_back(obj);
}

void PhysicsWorld::step(float dt) {
    applyGravityForces();
    // Friction/drag coefficient (tweak as needed)
    constexpr float friction = 0.08f; // Weaker drag for more persistent motion
    for (auto& obj : objects) {
        if (!obj.isStatic) {
            // Apply friction (simple linear drag)
            float v = std::sqrt(obj.vx * obj.vx + obj.vy * obj.vy);
            if (v > 1e-6f) {
                float drag = friction * dt;
                float scale = std::max(0.0f, v - drag) / v;
                obj.vx *= scale;
                obj.vy *= scale;
            }
            obj.vy += gravity * dt;
            obj.x += obj.vx * dt;
            obj.y += obj.vy * dt;
        }
    }
    handleCollisions();
    handleWalls();
}
// Universal gravitational constant (arbitrary scale for simulation)
constexpr float G = 0.5f;

void PhysicsWorld::applyGravityForces() {
    // Apply Newtonian gravity between all pairs
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = 0; j < objects.size(); ++j) {
            if (i == j) continue;
            PhysicsObject& a = objects[i];
            PhysicsObject& b = objects[j];
            if (a.isStatic || b.isStatic) continue; // Don't attract or be attracted if static
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < 1e-8f) continue; // Avoid division by zero and self-attraction
            float dist = std::sqrt(distSq) + 1e-6f;
            // F = G * m1 * m2 / r^2
            float F = G * a.mass * b.mass / distSq;
            float ax = F * dx / (dist * a.mass);
            float ay = F * dy / (dist * a.mass);
            a.vx += ax * 0.001f; // Reduce fudge factor for stability
            a.vy += ay * 0.001f;
        }
    }
}
void PhysicsWorld::handleWalls() {
    constexpr float wallDamping = 0.2f; // Lose 80% of velocity on wall hit
    for (auto& obj : objects) {
        if (obj.isStatic) continue;
        // Left wall
        if (obj.x - obj.radius < left) {
            obj.x = left + obj.radius;
            obj.vx = -obj.vx * wallDamping;
        }
        // Right wall
        if (obj.x + obj.radius > right) {
            obj.x = right - obj.radius;
            obj.vx = -obj.vx * wallDamping;
        }
        // Floor
        if (obj.y - obj.radius < bottom) {
            obj.y = bottom + obj.radius;
            obj.vy = -obj.vy * wallDamping;
        }
        // Ceiling
        if (obj.y + obj.radius > top) {
            obj.y = top - obj.radius;
            obj.vy = -obj.vy * wallDamping;
        }
    }
}

void PhysicsWorld::handleCollisions() {
    // Realistic collision for circles: use radii, separate on overlap, conserve momentum
    const float restitution = 0.95f; // More bounce for lively collisions
    const float percent = 0.2f; // Less positional correction for more natural collisions
    const float slop = 1e-4f;   // Small value to allow tiny overlap
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            PhysicsObject& a = objects[i];
            PhysicsObject& b = objects[j];
            if (a.isStatic && b.isStatic) continue;
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float distSq = dx * dx + dy * dy;
            float minDist = a.radius + b.radius;
            if (distSq < minDist * minDist) {
                float dist = std::sqrt(distSq) + 1e-8f;
                float nx = dx / dist;
                float ny = dy / dist;
                // --- Positional correction (Baumgarte stabilization) ---
                float penetration = minDist - dist;
                float correction = std::max(penetration - slop, 0.0f) / ((a.isStatic ? 0.0f : 1.0f) + (b.isStatic ? 0.0f : 1.0f)) * percent;
                if (!a.isStatic && !b.isStatic) {
                    a.x -= nx * correction * 0.5f;
                    a.y -= ny * correction * 0.5f;
                    b.x += nx * correction * 0.5f;
                    b.y += ny * correction * 0.5f;
                } else if (!a.isStatic) {
                    a.x -= nx * correction;
                    a.y -= ny * correction;
                } else if (!b.isStatic) {
                    b.x += nx * correction;
                    b.y += ny * correction;
                }

                // Project velocities onto normal and tangential directions
                float vax = a.vx, vay = a.vy;
                float vbx = b.vx, vby = b.vy;
                float van = vax * nx + vay * ny;
                float vbn = vbx * nx + vby * ny;
                float vat = -vax * ny + vay * nx;
                float vbt = -vbx * ny + vby * nx;
                float ma = a.isStatic ? 1e10f : a.mass;
                float mb = b.isStatic ? 1e10f : b.mass;
                // Only resolve if objects are moving toward each other (van - vbn > 0)
                if (van - vbn < 0.0f) continue;

                // --- Impulse-based collision resolution ---
                float relVel = van - vbn;
                float impulse = -(1.0f + restitution) * relVel / (1.0f / ma + 1.0f / mb);
                float impA = impulse / ma;
                float impB = impulse / mb;
                if (!a.isStatic) {
                    a.vx += impA * nx;
                    a.vy += impA * ny;
                }
                if (!b.isStatic) {
                    b.vx -= impB * nx;
                    b.vy -= impB * ny;
                }

                // Tangential velocity (friction, optional, can be added here)
                // Optionally, you can add a small tangential impulse for more realism
            }
        }
    }
}
