#include "physics.hpp"

#include <cmath>
#include <algorithm>

// Ensure the PhysicsWorld class is in scope
// If PhysicsWorld is in a namespace, add: using namespace <your_namespace>;

float PhysicsWorld::totalKineticEnergy() const {
    float ke = 0.0f;
    for (const auto& obj : this->objects) {
        if (!obj.isStatic) {
            if (obj.mass > 0 && std::isfinite(obj.vx) && std::isfinite(obj.vy)) {
                ke += 0.5f * obj.mass * (obj.vx * obj.vx + obj.vy * obj.vy);
            }
        }
    }
    return ke;
}

void PhysicsWorld::totalMomentum(float& px, float& py) const {
    px = 0.0f; py = 0.0f;
    for (const auto& obj : this->objects) {
        if (!obj.isStatic) {
            if (obj.mass > 0 && std::isfinite(obj.vx) && std::isfinite(obj.vy)) {
                px += obj.mass * obj.vx;
                py += obj.mass * obj.vy;
            }
        }
    }
}


void PhysicsWorld::addObject(const PhysicsObject& obj) {
    // Prevent objects from spawning inside existing objects
    for (const auto& existing : objects) {
        float dx = obj.x - existing.x;
        float dy = obj.y - existing.y;
        float minDist = obj.radius + existing.radius;
        if ((dx * dx + dy * dy) < (minDist * minDist)) {
            // Overlap detected, do not spawn
            return;
        }
    }
    objects.push_back(obj);
}

void PhysicsWorld::step(float dt) {
    // --- Continuous Collision Detection (CCD) for fast particles ---
    // (Simple: substep if any particle would move more than its radius in one step)
    float maxMove = 0.0f;
    for (const auto& obj : objects) {
        float move = std::sqrt(obj.vx * obj.vx + obj.vy * obj.vy) * dt;
        if (move > maxMove) maxMove = move;
    }
    int substeps = std::max(1, int(std::ceil(maxMove / 0.5f / dt)));
    float subdt = dt / substeps;
    for (int s = 0; s < substeps; ++s) {
        // ...existing code...
        // --- BlackHole absorption and Planet orbit logic ---
        // 1. BlackHole absorption: Remove objects within event horizon, increase BH mass
        std::vector<size_t> toAbsorb;
        for (size_t i = 0; i < objects.size(); ++i) {
            auto& obj = objects[i];
            if (obj.type == ObjectType::BlackHole) {
                for (size_t j = 0; j < objects.size(); ++j) {
                    if (i == j) continue;
                    auto& other = objects[j];
                    if (other.type == ObjectType::BlackHole) continue;
                    float dx = other.x - obj.x;
                    float dy = other.y - obj.y;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < obj.eventHorizon * obj.eventHorizon) {
                        // Absorb: add mass, conserve momentum, blend color
                        float totalMass = obj.mass + other.mass;
                        // Blend color by mass BEFORE updating obj.mass
                        obj.color.r = (obj.color.r * obj.mass + other.color.r * other.mass) / totalMass;
                        obj.color.g = (obj.color.g * obj.mass + other.color.g * other.mass) / totalMass;
                        obj.color.b = (obj.color.b * obj.mass + other.color.b * other.mass) / totalMass;
                        obj.vx = (obj.vx * obj.mass + other.vx * other.mass) / totalMass;
                        obj.vy = (obj.vy * obj.mass + other.vy * other.mass) / totalMass;
                        obj.x = (obj.x * obj.mass + other.x * other.mass) / totalMass;
                        obj.y = (obj.y * obj.mass + other.y * other.mass) / totalMass;
                        obj.mass = totalMass;
                        obj.radius = std::sqrt(obj.radius * obj.radius + other.radius * other.radius);
                        toAbsorb.push_back(j);
                    }
                }
            }
        }
        // Remove absorbed objects (from back to front)
        std::sort(toAbsorb.begin(), toAbsorb.end());
        toAbsorb.erase(std::unique(toAbsorb.begin(), toAbsorb.end()), toAbsorb.end());
        for (int k = static_cast<int>(toAbsorb.size()) - 1; k >= 0; --k) {
            size_t idx = toAbsorb[k];
            if (idx < objects.size()) objects.erase(objects.begin() + idx);
        }


        // 2. Planet orbit logic: update position/velocity for planets with valid orbitTarget
        for (auto& obj : objects) {
            if (obj.type == ObjectType::Planet && obj.orbitTarget >= 0 && obj.orbitTarget < (int)objects.size()) {
                const auto& target = objects[obj.orbitTarget];
                float angle = obj.orbitAngle;
                float r = obj.orbitRadius;
                obj.x = target.x + r * std::cos(angle);
                obj.y = target.y + r * std::sin(angle);
                // Set velocity for circular orbit (approximate)
                float v = std::sqrt(0.5f * target.mass / std::max(r, 1e-4f));
                obj.vx = -v * std::sin(angle) + target.vx;
                obj.vy =  v * std::cos(angle) + target.vy;
                // Advance orbit angle
                obj.orbitAngle += 0.01f; // Speed can be parameterized
            }
        }

        // 3. Visual rotation/spin logic: advance spin angle for all objects with nonzero spin
        for (auto& obj : objects) {
            // 'spin' is angular velocity in radians per second
            // 'spinAngle' is the current orientation (in radians)
            if (std::abs(obj.spin) > 1e-6f) {
                obj.spinAngle += obj.spin * subdt; // Time-step independent
                // Keep spinAngle in [0, 2pi) for numerical stability
                while (obj.spinAngle >= 2 * M_PI) obj.spinAngle -= 2 * M_PI;
                while (obj.spinAngle < 0) obj.spinAngle += 2 * M_PI;
            }
        }

        applyGravityForces();
        constexpr float friction = 0.08f;
        for (auto& obj : objects) {
            if (!obj.isStatic) {
                float v = std::sqrt(obj.vx * obj.vx + obj.vy * obj.vy);
                if (v > 1e-6f) {
                    float drag = friction * subdt;
                    float scale = std::max(0.0f, v - drag) / v;
                    obj.vx *= scale;
                    obj.vy *= scale;
                }
                obj.vy += gravity * subdt;
                obj.x += obj.vx * subdt;
                obj.y += obj.vy * subdt;
            }
        }
        handleCollisions();
        handleWalls();
    }
    return;
}
// Universal gravitational constant (scaled for simulation realism)
constexpr float G = 0.01f; // Lowered from 0.5f for more stable orbits and realistic gravity

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
    // --- Improved smooth collision logic (no merging or splitting) ---
    updateSpatialGrid();
    const float restitution = 0.95f;
    const float percent = 0.2f;
    const float slop = 1e-4f;
    // No object creation from collisions or interactions
    std::vector<std::pair<size_t, size_t>> checkedPairs;
    // Only check collisions within each cell and neighboring cells
    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {
            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    int nrow = row + di;
                    int ncol = col + dj;
                    if (nrow < 0 || nrow >= gridRows || ncol < 0 || ncol >= gridCols) continue;
                    const auto& cellA = gridCells[row][col];
                    const auto& cellB = gridCells[nrow][ncol];
                    for (size_t idxA = 0; idxA < cellA.size(); ++idxA) {
                        for (size_t idxB = 0; idxB < cellB.size(); ++idxB) {
                            size_t i = cellA[idxA];
                            size_t j = cellB[idxB];
                            if (i >= j) continue; // avoid double check and self
                            // Avoid duplicate checks
                            if (std::find(checkedPairs.begin(), checkedPairs.end(), std::make_pair(i, j)) != checkedPairs.end()) continue;
                            checkedPairs.push_back({i, j});
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
                                // --- Elastic collision (impulse-based) ---
                                float ma = a.isStatic ? 1e10f : a.mass;
                                float mb = b.isStatic ? 1e10f : b.mass;
                                float penetration = minDist - dist;
                                float correction = std::max(penetration - slop, 0.0f) / (ma + mb) * percent;
                                if (!a.isStatic && !b.isStatic) {
                                    a.x -= nx * correction * (mb / (ma + mb));
                                    a.y -= ny * correction * (mb / (ma + mb));
                                    b.x += nx * correction * (ma / (ma + mb));
                                    b.y += ny * correction * (ma / (ma + mb));
                                } else if (!a.isStatic) {
                                    a.x -= nx * correction;
                                    a.y -= ny * correction;
                                } else if (!b.isStatic) {
                                    b.x += nx * correction;
                                    b.y += ny * correction;
                                }
                                float vax = a.vx, vay = a.vy;
                                float vbx = b.vx, vby = b.vy;
                                float van = vax * nx + vay * ny;
                                float vbn = vbx * nx + vby * ny;
                                float relVel = van - vbn;
                                if (relVel < 0.0f) continue;
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
                            }
                        }
                    }
                }
            }
        }
    }
    // Merging and splitting fully removed
}

void PhysicsWorld::updateSpatialGrid() {
    // Compute cell size based on bounds and grid size
    cellWidth = (right - left) / gridCols;
    cellHeight = (top - bottom) / gridRows;
    // Resize grid
    gridCells.assign(gridRows, std::vector<std::vector<size_t>>(gridCols));
    // Assign each object to a cell
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& obj = objects[i];
        int col = static_cast<int>((obj.x - left) / cellWidth);
        int row = static_cast<int>((obj.y - bottom) / cellHeight);
        // Clamp to grid
        col = std::max(0, std::min(gridCols - 1, col));
        row = std::max(0, std::min(gridRows - 1, row));
        gridCells[row][col].push_back(i);
    }
}
