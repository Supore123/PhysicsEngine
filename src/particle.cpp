#include "particle.hpp"
#include <cmath>
#include <algorithm>

// Trail implementation
void Trail::addPoint(float x, float y) {
    positions.push_back(x);
    positions.push_back(y);
    if (positions.size() > maxPoints * 2) {
        positions.erase(positions.begin(), positions.begin() + 2);
    }
}

void Trail::update() {
    // Fade old points (could be used for alpha blending)
}

void Trail::clear() {
    positions.clear();
}

// Existing color function
Color3 ParticleUtils::getColor(const Particle& p, float minMass, float maxMass) {
    if (p.type == ObjectType::Merged)
        return {0.2f, 0.8f, 1.0f};
    
    float norm = (std::log10(p.mass) - std::log10(minMass)) / 
                 (std::log10(maxMass) - std::log10(minMass));
    norm = std::max(0.0f, std::min(1.0f, norm));
    
    // Jet colormap
    float r, g, b;
    if (norm < 0.25f) {
        float t = norm / 0.25f;
        r = 0.9f;
        g = 0.9f * t + 0.1f * (1.0f - t);
        b = 1.0f;
    } else if (norm < 0.5f) {
        float t = (norm - 0.25f) / 0.25f;
        r = 1.0f;
        g = 1.0f;
        b = 1.0f - t;
    } else if (norm < 0.65f) {
        float t = (norm - 0.5f) / 0.15f;
        r = 1.0f;
        g = 1.0f - t;
        b = 0.0f;
    } else if (norm < 0.7f) {
        float t = (norm - 0.65f) / 0.05f;
        r = 1.0f;
        g = 0.85f * (1.0f - t);
        b = 0.0f;
    } else {
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
    }
    return {r, g, b};
}

void ParticleUtils::computeMassRange(const std::vector<Particle>& particles, 
                                     float& minMass, float& maxMass) {
    minMass = 1e6f;
    maxMass = -1e6f;
    for (const auto& p : particles) {
        if (p.type == ObjectType::Merged) continue;
        if (p.mass < minMass) minMass = p.mass;
        if (p.mass > maxMass) maxMass = p.mass;
    }
    if (minMass > maxMass) {
        minMass = 1.0f;
        maxMass = 10.0f;
    }
}

float ParticleUtils::getSize(const Particle& p) {
    float baseMult = 600.0f;
    switch (p.type) {
        case ObjectType::BlackHole: return p.radius * 500.0f;
        case ObjectType::NeutronStar: return p.radius * 700.0f;
        case ObjectType::Star: return p.radius * 650.0f;
        case ObjectType::GasGiant: return p.radius * 800.0f;
        case ObjectType::Planet:
        case ObjectType::RockyPlanet: return p.radius * 600.0f;
        case ObjectType::Asteroid: return p.radius * 400.0f;
        case ObjectType::Comet: return p.radius * 500.0f;
        case ObjectType::Merged: return p.radius * 800.0f;
        default: return p.radius * baseMult;
    }
}

// NEW: Temperature-based color for stars
Color3 ParticleUtils::getTemperatureColor(float temp) {
    // Blackbody radiation approximation
    // Cool red dwarfs: ~2000-3500K
    // Sun-like: ~5000-6000K
    // Hot blue stars: ~10000K+
    
    if (temp < 3500.0f) {
        // Red to orange
        float t = (temp - 2000.0f) / 1500.0f;
        return {1.0f, 0.3f + t * 0.4f, 0.0f};
    } else if (temp < 5500.0f) {
        // Orange to yellow
        float t = (temp - 3500.0f) / 2000.0f;
        return {1.0f, 0.7f + t * 0.3f, t * 0.3f};
    } else if (temp < 7500.0f) {
        // Yellow to white
        float t = (temp - 5500.0f) / 2000.0f;
        return {1.0f, 1.0f, 0.3f + t * 0.7f};
    } else {
        // White to blue
        float t = std::min((temp - 7500.0f) / 5000.0f, 1.0f);
        return {1.0f - t * 0.3f, 1.0f - t * 0.2f, 1.0f};
    }
}

// NEW: Create preset star
Particle ParticleUtils::createStar(float x, float y, float mass, float temp) {
    Particle star;
    star.x = x;
    star.y = y;
    star.vx = star.vy = 0.0f;
    star.radius = 0.04f + mass * 0.003f;
    star.mass = mass;
    star.type = ObjectType::Star;
    star.temperature = temp;
    star.emitsLight = true;
    star.luminosity = mass * 0.1f;
    star.color = getTemperatureColor(temp);
    star.spin = 0.5f + mass * 0.05f;
    star.density = 1.4f; // Solar density
    return star;
}

// NEW: Create preset planet
Particle ParticleUtils::createPlanet(float x, float y, float radius, float mass, bool gasGiant) {
    Particle planet;
    planet.x = x;
    planet.y = y;
    planet.vx = planet.vy = 0.0f;
    planet.radius = radius;
    planet.mass = mass;
    planet.type = gasGiant ? ObjectType::GasGiant : ObjectType::RockyPlanet;
    planet.temperature = 273.0f;
    planet.density = gasGiant ? 0.7f : 5.5f;
    planet.spin = gasGiant ? 3.0f : 1.0f;
    
    // Set appropriate colors
    if (gasGiant) {
        planet.color = {0.8f, 0.7f, 0.5f};
    } else {
        planet.color = {0.5f, 0.5f, 0.8f};
    }
    
    return planet;
}

// NEW: Create black hole
Particle ParticleUtils::createBlackHole(float x, float y, float mass) {
    Particle bh;
    bh.x = x;
    bh.y = y;
    bh.vx = bh.vy = 0.0f;
    bh.radius = 0.04f + mass * 0.002f;
    bh.mass = mass;
    bh.type = ObjectType::BlackHole;
    bh.eventHorizon = bh.schwarzschildRadius() * 2.0f;
    bh.absorption = 1.0f;
    bh.color = {0.05f, 0.05f, 0.1f};
    bh.spin = 5.0f;
    bh.density = 1000.0f;
    bh.temperature = 2.7f; // Hawking radiation
    return bh;
}

// NEW: Create comet
Particle ParticleUtils::createComet(float x, float y, float vx, float vy) {
    Particle comet;
    comet.x = x;
    comet.y = y;
    comet.vx = vx;
    comet.vy = vy;
    comet.radius = 0.012f;
    comet.mass = 0.1f;
    comet.type = ObjectType::Comet;
    comet.color = {0.7f, 0.8f, 0.9f};
    comet.spin = 2.0f;
    comet.density = 0.5f;
    
    // Create trail
    comet.trail = new Trail();
    comet.trail->maxPoints = 150;
    
    return comet;
}

// NEW: Create neutron star
Particle ParticleUtils::createNeutronStar(float x, float y, float mass) {
    Particle ns;
    ns.x = x;
    ns.y = y;
    ns.vx = ns.vy = 0.0f;
    ns.radius = 0.015f; // Very small
    ns.mass = mass;
    ns.type = ObjectType::NeutronStar;
    ns.temperature = 1000000.0f; // Extremely hot
    ns.emitsLight = true;
    ns.luminosity = 0.5f;
    ns.color = {0.8f, 0.9f, 1.0f};
    ns.spin = 30.0f; // Rapidly spinning
    ns.density = 10000.0f; // Incredibly dense
    ns.magneticField = 100.0f; // Strong magnetic field
    return ns;
}