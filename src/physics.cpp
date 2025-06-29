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
    // --- Hybrid collision/merge/split logic ---
    const float mergeSpeedThreshold = 0.5f; // Tune as needed
    std::vector<size_t> toRemove;
    struct MergeEvent { size_t i, j; };
    struct SplitEvent { size_t mergedIdx, hitterIdx; };
    std::vector<MergeEvent> merges;
    std::vector<MergeEvent> bounces;
    std::vector<SplitEvent> splits;
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            PhysicsObject& a = objects[i];
            PhysicsObject& b = objects[j];
            if (a.isStatic && b.isStatic) continue;
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float distSq = dx * dx + dy * dy;
            float minDist = a.radius + b.radius - 0.01f;
            if (distSq < minDist * minDist) {
                float dist = std::sqrt(distSq) + 1e-8f;
                float nx = dx / dist;
                float ny = dy / dist;
                float relVx = b.vx - a.vx;
                float relVy = b.vy - a.vy;
                float relSpeed = relVx * nx + relVy * ny;
                // If both are normal and slow, merge
                if (a.type == ObjectType::Normal && b.type == ObjectType::Normal && std::abs(relSpeed) < mergeSpeedThreshold) {
                    merges.push_back({i, j});
                } else if (a.type == ObjectType::Normal && b.type == ObjectType::Normal) {
                    bounces.push_back({i, j});
                } else if ((a.type == ObjectType::Merged && b.type == ObjectType::Normal) && std::abs(relSpeed) > mergeSpeedThreshold) {
                    splits.push_back({i, j});
                } else if ((a.type == ObjectType::Normal && b.type == ObjectType::Merged) && std::abs(relSpeed) > mergeSpeedThreshold) {
                    splits.push_back({j, i});
                } else if ((a.type == ObjectType::Merged && b.type == ObjectType::Normal) || (a.type == ObjectType::Normal && b.type == ObjectType::Merged)) {
                    // Slow hit: merge normal into merged
                    merges.push_back({i, j});
                } else if (a.type == ObjectType::Merged && b.type == ObjectType::Merged) {
                    // Merged-merged: always bounce for now
                    bounces.push_back({i, j});
                }
            }
        }
    }
    // Handle merges
    std::vector<bool> merged(objects.size(), false);
    for (const auto& m : merges) {
        if (merged[m.i] || merged[m.j]) continue;
        PhysicsObject& a = objects[m.i];
        PhysicsObject& b = objects[m.j];
        float totalMass = a.mass + b.mass;
        float x = (a.x * a.mass + b.x * b.mass) / totalMass;
        float y = (a.y * a.mass + b.y * b.mass) / totalMass;
        float vx = (a.vx * a.mass + b.vx * b.mass) / totalMass;
        float vy = (a.vy * a.mass + b.vy * b.mass) / totalMass;
        float newRadius = std::sqrt(a.radius * a.radius + b.radius * b.radius) * 1.3f;
        PhysicsObject mergedObj = {x, y, vx, vy, newRadius, totalMass, 0.0f, false, ObjectType::Merged};
        // Store components
        if (a.type == ObjectType::Merged) mergedObj.components = a.components; else mergedObj.components = {a};
        if (b.type == ObjectType::Merged) mergedObj.components.insert(mergedObj.components.end(), b.components.begin(), b.components.end()); else mergedObj.components.push_back(b);
        merged[m.i] = true;
        merged[m.j] = true;
        objects.push_back(mergedObj);
    }
    // Handle splits
    for (const auto& s : splits) {
        PhysicsObject& mergedObj = objects[s.mergedIdx];
        if (mergedObj.type != ObjectType::Merged) continue;
        // Place all components at mergedObj's position with a small velocity kick
        float kick = 0.5f;
        for (auto& part : mergedObj.components) {
            part.x = mergedObj.x;
            part.y = mergedObj.y;
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
            part.vx = mergedObj.vx + std::cos(angle) * kick;
            part.vy = mergedObj.vy + std::sin(angle) * kick;
            objects.push_back(part);
        }
        merged[s.mergedIdx] = true;
        // Optionally, remove the hitter too (simulate absorption)
        // merged[s.hitterIdx] = true;
    }
    // Handle bounces (impulse-based)
    for (const auto& b : bounces) {
        if (merged[b.i] || merged[b.j]) continue;
        PhysicsObject& a = objects[b.i];
        PhysicsObject& bobj = objects[b.j];
        float dx = bobj.x - a.x;
        float dy = bobj.y - a.y;
        float dist = std::sqrt(dx * dx + dy * dy) + 1e-8f;
        float nx = dx / dist;
        float ny = dy / dist;
        float penetration = (a.radius + bobj.radius - 0.01f) - dist;
        float correction = std::max(penetration - slop, 0.0f) / ((a.isStatic ? 0.0f : 1.0f) + (bobj.isStatic ? 0.0f : 1.0f)) * percent;
        if (!a.isStatic && !bobj.isStatic) {
            a.x -= nx * correction * 0.5f;
            a.y -= ny * correction * 0.5f;
            bobj.x += nx * correction * 0.5f;
            bobj.y += ny * correction * 0.5f;
        } else if (!a.isStatic) {
            a.x -= nx * correction;
            a.y -= ny * correction;
        } else if (!bobj.isStatic) {
            bobj.x += nx * correction;
            bobj.y += ny * correction;
        }
        float vax = a.vx, vay = a.vy;
        float vbx = bobj.vx, vby = bobj.vy;
        float van = vax * nx + vay * ny;
        float vbn = vbx * nx + vby * ny;
        float ma = a.isStatic ? 1e10f : a.mass;
        float mb = bobj.isStatic ? 1e10f : bobj.mass;
        if (van - vbn < 0.0f) continue;
        float relVel = van - vbn;
        float impulse = -(1.0f + restitution) * relVel / (1.0f / ma + 1.0f / mb);
        float impA = impulse / ma;
        float impB = impulse / mb;
        if (!a.isStatic) {
            a.vx += impA * nx;
            a.vy += impA * ny;
        }
        if (!bobj.isStatic) {
            bobj.vx -= impB * nx;
            bobj.vy -= impB * ny;
        }
    }
    // Remove merged/split objects (from back to front)
    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(merged.size()) && merged[i]) {
            objects.erase(objects.begin() + i);
        }
    }
}
