#include "physics.hpp"
#include <cmath>
#include <algorithm>
#include <random>

// Universal gravitational constant
constexpr float G = 0.01f;

float PhysicsWorld::totalKineticEnergy() const {
    float ke = 0.0f;
    for (const auto& obj : objects) {
        if (!obj.isStatic && obj.mass > 0 && std::isfinite(obj.vx) && std::isfinite(obj.vy)) {
            ke += 0.5f * obj.mass * (obj.vx * obj.vx + obj.vy * obj.vy);
        }
    }
    return ke;
}

void PhysicsWorld::totalMomentum(float& px, float& py) const {
    px = 0.0f; py = 0.0f;
    for (const auto& obj : objects) {
        if (!obj.isStatic && obj.mass > 0 && std::isfinite(obj.vx) && std::isfinite(obj.vy)) {
            px += obj.mass * obj.vx;
            py += obj.mass * obj.vy;
        }
    }
}

float PhysicsWorld::totalPotentialEnergy() const {
    float pe = 0.0f;
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            float dx = objects[j].x - objects[i].x;
            float dy = objects[j].y - objects[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);
            pe -= G * objects[i].mass * objects[j].mass / std::max(dist, 1e-4f);
        }
    }
    return pe;
}

float PhysicsWorld::totalAngularMomentum() const {
    float L = 0.0f;
    for (const auto& obj : objects) {
        if (!obj.isStatic) {
            L += obj.x * obj.mass * obj.vy - obj.y * obj.mass * obj.vx;
        }
    }
    return L;
}

void PhysicsWorld::addObject(const PhysicsObject& obj) {
    for (const auto& existing : objects) {
        float dx = obj.x - existing.x;
        float dy = obj.y - existing.y;
        float minDist = obj.radius + existing.radius;
        if ((dx * dx + dy * dy) < (minDist * minDist)) {
            return;
        }
    }
    objects.push_back(obj);
}

void PhysicsWorld::applyForceFields() {
    for (auto& field : forceFields) {
        if (!field.active) continue;
        
        for (auto& obj : objects) {
            if (obj.isStatic) continue;
            
            float dx = obj.x - field.x;
            float dy = obj.y - field.y;
            float distSq = dx * dx + dy * dy;
            float dist = std::sqrt(distSq) + 1e-6f;
            
            if (dist > field.radius) continue;
            
            float falloff = 1.0f - (dist / field.radius);
            
            switch (field.type) {
                case ForceField::RADIAL: {
                    float fx = (dx / dist) * field.strength * falloff;
                    float fy = (dy / dist) * field.strength * falloff;
                    obj.vx += fx * 0.01f;
                    obj.vy += fy * 0.01f;
                    break;
                }
                case ForceField::VORTEX: {
                    float tangentX = -dy / dist;
                    float tangentY = dx / dist;
                    obj.vx += tangentX * field.strength * falloff * 0.01f;
                    obj.vy += tangentY * field.strength * falloff * 0.01f;
                    obj.vx -= (dx / dist) * field.strength * falloff * 0.002f;
                    obj.vy -= (dy / dist) * field.strength * falloff * 0.002f;
                    break;
                }
                case ForceField::DIRECTIONAL: {
                    float fx = std::cos(field.angle) * field.strength * falloff;
                    float fy = std::sin(field.angle) * field.strength * falloff;
                    obj.vx += fx * 0.01f;
                    obj.vy += fy * 0.01f;
                    break;
                }
                case ForceField::CUSTOM: {
                    if (field.customForce) {
                        field.customForce(obj, dx, dy);
                    }
                    break;
                }
            }
        }
    }
}

void PhysicsWorld::applyAirDrag(float dt) {
    if (airDragCoefficient <= 0.0f) return;
    
    for (auto& obj : objects) {
        if (obj.isStatic) continue;
        
        float speed = std::sqrt(obj.vx * obj.vx + obj.vy * obj.vy);
        if (speed < 1e-6f) continue;
        
        float dragForce = airDragCoefficient * speed * speed * obj.radius;
        float ax = -(obj.vx / speed) * dragForce / obj.mass;
        float ay = -(obj.vy / speed) * dragForce / obj.mass;
        
        obj.vx += ax * dt;
        obj.vy += ay * dt;
    }
}

void PhysicsWorld::applyTidalForces() {
    for (size_t i = 0; i < objects.size(); ++i) {
        auto& obj = objects[i];
        if (obj.type != ObjectType::Planet && obj.type != ObjectType::RockyPlanet) continue;
        
        size_t closestIdx = 0;
        float minDist = 1e10f;
        for (size_t j = 0; j < objects.size(); ++j) {
            if (i == j) continue;
            if (objects[j].mass < obj.mass * 5.0f) continue;
            
            float dx = objects[j].x - obj.x;
            float dy = objects[j].y - obj.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                closestIdx = j;
            }
        }
        
        if (minDist < 1e9f) {
            const auto& massive = objects[closestIdx];
            float tidalR = getTidalRadius(obj, massive);
            
            if (minDist < tidalR) {
                if (obj.mass > 0.1f) {
                    createDebrisField(obj.x, obj.y, 8, 0.02f);
                    objects.erase(objects.begin() + i);
                    --i;
                }
            }
            
            if (obj.tidallyLocked && obj.orbitTarget == (int)closestIdx) {
                float angle = std::atan2(massive.y - obj.y, massive.x - obj.x);
                obj.spinAngle = angle;
                obj.spin = 0.0f;
            }
        }
    }
}

void PhysicsWorld::updateTemperatures(float dt) {
    for (auto& obj : objects) {
        if (obj.temperature > 273.0f) {
            obj.temperature -= dt * 0.1f;
        }
        
        if (obj.type == ObjectType::Star) {
            obj.temperature = 2000.0f + obj.mass * 300.0f;
            obj.emitsLight = true;
        }
        
        for (const auto& other : objects) {
            if (other.emitsLight && &other != &obj) {
                float dx = other.x - obj.x;
                float dy = other.y - obj.y;
                float distSq = dx * dx + dy * dy;
                float dist = std::sqrt(distSq);
                if (dist < 0.5f) {
                    float heating = other.luminosity / (distSq + 0.01f) * dt;
                    obj.temperature += heating * 0.5f;
                }
            }
        }
    }
}

void PhysicsWorld::handleSupernova(size_t starIndex) {
    if (starIndex >= objects.size()) return;
    
    auto& star = objects[starIndex];
    if (star.type != ObjectType::Star) return;
    
    float explosionEnergy = star.mass * 10.0f;
    createDebrisField(star.x, star.y, 50, 0.15f);
    
    for (auto& obj : objects) {
        if (&obj == &star) continue;
        float dx = obj.x - star.x;
        float dy = obj.y - star.y;
        float distSq = dx * dx + dy * dy;
        float dist = std::sqrt(distSq);
        if (dist < 0.8f) {
            float force = explosionEnergy / (distSq + 0.01f);
            float norm = dist + 1e-6f;
            obj.vx += (dx / norm) * force * 0.01f;
            obj.vy += (dy / norm) * force * 0.01f;
        }
    }
    
    if (star.mass > 8.0f) {
        star.type = ObjectType::BlackHole;
        star.mass *= 0.3f;
        star.radius *= 0.3f;
        star.eventHorizon = star.mass * 0.002f * 1.5f;
        star.color = {0.1f, 0.1f, 0.1f};
        star.spin = 10.0f;
    } else {
        star.type = ObjectType::NeutronStar;
        star.mass *= 0.5f;
        star.radius *= 0.2f;
        star.color = {0.8f, 0.8f, 1.0f};
        star.spin = 20.0f;
    }
}

void PhysicsWorld::createDebrisField(float x, float y, int count, float speed) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> speedDist(speed * 0.5f, speed * 1.5f);
    
    for (int i = 0; i < count; ++i) {
        float angle = angleDist(gen);
        float s = speedDist(gen);
        
        Particle debris;
        debris.x = x;
        debris.y = y;
        debris.vx = std::cos(angle) * s;
        debris.vy = std::sin(angle) * s;
        debris.radius = 0.008f;
        debris.mass = 0.05f;
        debris.type = ObjectType::Asteroid;
        debris.color = {0.6f, 0.5f, 0.4f};
        debris.spin = angleDist(gen) * 5.0f;
        debris.lifetime = 30.0f;
        
        objects.push_back(debris);
    }
}

void PhysicsWorld::addForceField(const ForceField& field) {
    forceFields.push_back(field);
}

void PhysicsWorld::removeForceField(size_t index) {
    if (index < forceFields.size()) {
        forceFields.erase(forceFields.begin() + index);
    }
}

void PhysicsWorld::clearForceFields() {
    forceFields.clear();
}

void PhysicsWorld::createGalaxy(float centerX, float centerY, int armCount, int starsPerArm) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> offsetDist(-0.05f, 0.05f);
    
    // Central black hole
    Particle bh;
    bh.x = centerX;
    bh.y = centerY;
    bh.mass = 30.0f;
    bh.radius = 0.05f;
    bh.type = ObjectType::BlackHole;
    bh.eventHorizon = 0.08f;
    bh.isStatic = true;
    bh.color = {0.1f, 0.1f, 0.1f};
    bh.spin = 8.0f;
    addObject(bh);
    
    // Spiral arms
    for (int arm = 0; arm < armCount; ++arm) {
        float baseAngle = (2.0f * M_PI * arm) / armCount;
        
        for (int i = 0; i < starsPerArm; ++i) {
            float t = float(i) / starsPerArm;
            float radius = 0.15f + t * 0.6f;
            float spiralAngle = baseAngle + t * 2.0f * M_PI;
            
            float x = centerX + radius * std::cos(spiralAngle) + offsetDist(gen);
            float y = centerY + radius * std::sin(spiralAngle) + offsetDist(gen);
            
            Particle star;
            star.x = x;
            star.y = y;
            star.radius = 0.012f;
            star.mass = 0.5f + t * 2.0f;
            star.type = ObjectType::Star;
            star.temperature = 3000.0f + t * 5000.0f;
            star.emitsLight = true;
            star.luminosity = 0.3f;
            star.color = {0.9f + t * 0.1f, 0.8f, 0.6f - t * 0.4f};
            star.spin = 1.0f;
            
            // Orbital velocity
            float v = std::sqrt(G * bh.mass / std::max(radius, 0.1f)) * 0.8f;
            star.vx = -v * std::sin(spiralAngle);
            star.vy = v * std::cos(spiralAngle);
            
            addObject(star);
        }
    }
}

void PhysicsWorld::createAsteroidBelt(float centerX, float centerY, float innerR, float outerR, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> radiusDist(innerR, outerR);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    
    for (int i = 0; i < count; ++i) {
        float r = radiusDist(gen);
        float angle = angleDist(gen);
        
        Particle ast;
        ast.x = centerX + r * std::cos(angle);
        ast.y = centerY + r * std::sin(angle);
        ast.radius = 0.01f;
        ast.mass = 0.1f;
        ast.type = ObjectType::Asteroid;
        ast.color = {0.6f, 0.5f, 0.4f};
        ast.spin = angleDist(gen) * 3.0f;
        
        // Find central massive object for orbital velocity
        float centralMass = 10.0f;
        for (const auto& obj : objects) {
            float dx = obj.x - centerX;
            float dy = obj.y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 0.1f && obj.mass > centralMass) {
                centralMass = obj.mass;
            }
        }
        
        float v = std::sqrt(G * centralMass / std::max(r, 0.1f));
        ast.vx = -v * std::sin(angle);
        ast.vy = v * std::cos(angle);
        
        addObject(ast);
    }
}

void PhysicsWorld::step(float dt) {
    // CCD: Check max movement
    float maxMove = 0.0f;
    for (const auto& obj : objects) {
        float move = std::sqrt(obj.vx * obj.vx + obj.vy * obj.vy) * dt;
        if (move > maxMove) maxMove = move;
    }
    int substeps = std::max(1, int(std::ceil(maxMove / 0.5f / dt)));
    float subdt = dt / substeps;
    
    for (int s = 0; s < substeps; ++s) {
        // Black hole absorption
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
                        float totalMass = obj.mass + other.mass;
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
                        stats.objectsAbsorbed++;
                    }
                }
            }
        }
        
        std::sort(toAbsorb.begin(), toAbsorb.end());
        toAbsorb.erase(std::unique(toAbsorb.begin(), toAbsorb.end()), toAbsorb.end());
        for (int k = static_cast<int>(toAbsorb.size()) - 1; k >= 0; --k) {
            size_t idx = toAbsorb[k];
            if (idx < objects.size()) objects.erase(objects.begin() + idx);
        }
        
        // Planet orbits
        for (auto& obj : objects) {
            if (obj.type == ObjectType::Planet && obj.orbitTarget >= 0 && obj.orbitTarget < (int)objects.size()) {
                const auto& target = objects[obj.orbitTarget];
                float angle = obj.orbitAngle;
                float r = obj.orbitRadius;
                obj.x = target.x + r * std::cos(angle);
                obj.y = target.y + r * std::sin(angle);
                float v = std::sqrt(0.5f * target.mass / std::max(r, 1e-4f));
                obj.vx = -v * std::sin(angle) + target.vx;
                obj.vy = v * std::cos(angle) + target.vy;
                obj.orbitAngle += 0.01f;
            }
        }
        
        // Spin
        for (auto& obj : objects) {
            if (std::abs(obj.spin) > 1e-6f) {
                obj.spinAngle += obj.spin * subdt;
                while (obj.spinAngle >= 2 * M_PI) obj.spinAngle -= 2 * M_PI;
                while (obj.spinAngle < 0) obj.spinAngle += 2 * M_PI;
            }
        }
        
        // Age and lifetime
        for (size_t i = 0; i < objects.size(); ++i) {
            objects[i].updateAge(subdt);
            if (objects[i].decaying) {
                if (objects[i].type == ObjectType::Star) {
                    handleSupernova(i);
                } else {
                    objects.erase(objects.begin() + i);
                    --i;
                }
            }
        }
        
        applyGravityForces();
        applyForceFields();
        applyAirDrag(subdt);
        
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
                
                // Update trail
                if (obj.trail && obj.type == ObjectType::Comet) {
                    obj.trail->addPoint(obj.x, obj.y);
                }
            }
        }
        
        handleCollisions();
        handleWalls();
    }
    
    applyTidalForces();
    updateTemperatures(dt);
}

void PhysicsWorld::applyGravityForces() {
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = 0; j < objects.size(); ++j) {
            if (i == j) continue;
            PhysicsObject& a = objects[i];
            PhysicsObject& b = objects[j];
            if (a.isStatic || b.isStatic) continue;
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < 1e-8f) continue;
            float dist = std::sqrt(distSq) + 1e-6f;
            float F = G * a.mass * b.mass / distSq;
            float ax = F * dx / (dist * a.mass);
            float ay = F * dy / (dist * a.mass);
            a.vx += ax * 0.001f;
            a.vy += ay * 0.001f;
        }
    }
}

void PhysicsWorld::handleWalls() {
    constexpr float wallDamping = 0.2f;
    for (auto& obj : objects) {
        if (obj.isStatic) continue;
        if (obj.x - obj.radius < left) {
            obj.x = left + obj.radius;
            obj.vx = -obj.vx * wallDamping;
        }
        if (obj.x + obj.radius > right) {
            obj.x = right - obj.radius;
            obj.vx = -obj.vx * wallDamping;
        }
        if (obj.y - obj.radius < bottom) {
            obj.y = bottom + obj.radius;
            obj.vy = -obj.vy * wallDamping;
        }
        if (obj.y + obj.radius > top) {
            obj.y = top - obj.radius;
            obj.vy = -obj.vy * wallDamping;
        }
    }
}

void PhysicsWorld::handleCollisions() {
    updateSpatialGrid();
    const float restitution = 0.95f;
    const float percent = 0.2f;
    const float slop = 1e-4f;
    
    std::vector<std::pair<size_t, size_t>> checkedPairs;
    
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
                            if (i >= j) continue;
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
                                stats.totalCollisions++;
                                float dist = std::sqrt(distSq) + 1e-8f;
                                float nx = dx / dist;
                                float ny = dy / dist;
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
}

void PhysicsWorld::updateSpatialGrid() {
    cellWidth = (right - left) / gridCols;
    cellHeight = (top - bottom) / gridRows;
    gridCells.assign(gridRows, std::vector<std::vector<size_t>>(gridCols));
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& obj = objects[i];
        int col = static_cast<int>((obj.x - left) / cellWidth);
        int row = static_cast<int>((obj.y - bottom) / cellHeight);
        col = std::max(0, std::min(gridCols - 1, col));
        row = std::max(0, std::min(gridRows - 1, row));
        gridCells[row][col].push_back(i);
    }
}