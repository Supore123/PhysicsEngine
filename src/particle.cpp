
#include "particle.hpp"
#include <cmath>


// Jet-like color map for mass (light for small, red for massive)
Color3 ParticleUtils::getColor(const Particle& p, float minMass, float maxMass) {
    if (p.type == ObjectType::Merged)
        return {0.2f, 0.8f, 1.0f}; // Cyan for merged
    float norm = (std::log10(p.mass) - std::log10(minMass)) / (std::log10(maxMass) - std::log10(minMass));
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
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

void ParticleUtils::computeMassRange(const std::vector<Particle>& particles, float& minMass, float& maxMass) {
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
    if (p.type == ObjectType::Merged)
        return p.radius * 800.0f;
    else
        return p.radius * 600.0f;
}
