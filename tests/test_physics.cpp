// Unit tests for PhysicsEngine - written for Issue #2
// Using Catch2 single-header (v2.x)
//
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "physics.hpp"
#include "particle.hpp"
#include <cmath>

// --- helpers ---

static Particle makeParticle(float x, float y, float vx = 0, float vy = 0,
                               float radius = 0.05f, float mass = 1.0f) {
    Particle p;
    p.x = x; p.y = y;
    p.vx = vx; p.vy = vy;
    p.radius = radius;
    p.mass = mass;
    p.color = {1.f, 0.f, 0.f};
    return p;
}

// floating point comparison with tolerance
static bool approxEq(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) < eps;
}


// ============================================================
// Particle struct
// ============================================================

TEST_CASE("Particle – default construction", "[particle]") {
    Particle p;
    REQUIRE(p.vx == 0.0f);
    REQUIRE(p.vy == 0.0f);
    REQUIRE(p.isStatic == false);
    REQUIRE(p.type == ObjectType::Normal);
    REQUIRE(p.lifetime == -1.0f);
    REQUIRE(p.age == 0.0f);
    REQUIRE(p.decaying == false);
    REQUIRE(p.restitution == Approx(0.95f));
}

TEST_CASE("Particle – kineticEnergy", "[particle]") {
    // KE = 0.5 * 2 * (9 + 16) = 25
    Particle p = makeParticle(0, 0, 3.0f, 4.0f, 0.05f, 2.0f);
    REQUIRE(approxEq(p.kineticEnergy(), 25.0f));
}

TEST_CASE("Particle – kineticEnergy is zero for static objects", "[particle]") {
    Particle p = makeParticle(0, 0, 5.0f, 5.0f);
    p.isStatic = true;
    REQUIRE(p.kineticEnergy() == 0.0f);
}

TEST_CASE("Particle – distanceTo", "[particle]") {
    Particle a = makeParticle(0, 0);
    Particle b = makeParticle(3, 4);
    REQUIRE(approxEq(a.distanceTo(b), 5.0f));
    REQUIRE(approxEq(b.distanceTo(a), 5.0f));
}

TEST_CASE("Particle – distanceTo self is zero", "[particle]") {
    Particle a = makeParticle(1.5f, -2.3f);
    REQUIRE(approxEq(a.distanceTo(a), 0.0f));
}

TEST_CASE("Particle – updateAge and decaying flag", "[particle]") {
    Particle p = makeParticle(0, 0);
    p.lifetime = 1.0f;

    p.updateAge(0.5f);
    REQUIRE(approxEq(p.age, 0.5f));
    REQUIRE_FALSE(p.decaying);

    p.updateAge(0.6f); // age = 1.1, should trigger decay
    REQUIRE(p.decaying);
}

TEST_CASE("Particle – infinite lifetime never triggers decay", "[particle]") {
    Particle p = makeParticle(0, 0);
    p.lifetime = -1.0f;
    p.updateAge(1000.0f);
    REQUIRE_FALSE(p.decaying);
}

TEST_CASE("Particle – schwarzschildRadius scales linearly with mass", "[particle]") {
    Particle bh1 = makeParticle(0, 0, 0, 0, 0.1f, 10.0f);
    Particle bh2 = makeParticle(0, 0, 0, 0, 0.1f, 20.0f);
    REQUIRE(bh2.schwarzschildRadius() == Approx(bh1.schwarzschildRadius() * 2.0f));
}


// ============================================================
// ParticleUtils
// ============================================================

TEST_CASE("ParticleUtils – computeMassRange basic", "[particle][utils]") {
    std::vector<Particle> particles;
    particles.push_back(makeParticle(0, 0, 0, 0, 0.05f, 1.0f));
    particles.push_back(makeParticle(0, 0, 0, 0, 0.05f, 5.0f));
    particles.push_back(makeParticle(0, 0, 0, 0, 0.05f, 10.0f));

    float minM, maxM;
    ParticleUtils::computeMassRange(particles, minM, maxM);
    REQUIRE(approxEq(minM, 1.0f));
    REQUIRE(approxEq(maxM, 10.0f));
}

TEST_CASE("ParticleUtils – computeMassRange empty list gives safe defaults", "[particle][utils]") {
    std::vector<Particle> empty;
    float minM = 0, maxM = 0;
    ParticleUtils::computeMassRange(empty, minM, maxM);
    REQUIRE(minM <= maxM);
}

TEST_CASE("ParticleUtils – computeMassRange skips Merged objects", "[particle][utils]") {
    std::vector<Particle> particles;

    Particle merged = makeParticle(0, 0, 0, 0, 0.05f, 999.0f);
    merged.type = ObjectType::Merged;
    particles.push_back(merged);

    particles.push_back(makeParticle(0, 0, 0, 0, 0.05f, 2.0f));

    float minM, maxM;
    ParticleUtils::computeMassRange(particles, minM, maxM);
    REQUIRE(approxEq(minM, 2.0f));
    REQUIRE(approxEq(maxM, 2.0f));
}

TEST_CASE("ParticleUtils – temperature colour: cool star is reddish", "[particle][utils]") {
    Color3 c = ParticleUtils::getTemperatureColor(2500.0f);
    REQUIRE(c.r >= c.b);
}

TEST_CASE("ParticleUtils – temperature colour: hot star has high blue channel", "[particle][utils]") {
    Color3 c = ParticleUtils::getTemperatureColor(15000.0f);
    REQUIRE(c.b >= 0.9f);
}

TEST_CASE("ParticleUtils – createStar", "[particle][utils]") {
    Particle s = ParticleUtils::createStar(0.1f, 0.2f, 5.0f, 5778.0f);
    REQUIRE(s.type == ObjectType::Star);
    REQUIRE(s.emitsLight);
    REQUIRE(approxEq(s.x, 0.1f));
    REQUIRE(approxEq(s.y, 0.2f));
}

TEST_CASE("ParticleUtils – createBlackHole", "[particle][utils]") {
    Particle bh = ParticleUtils::createBlackHole(0.0f, 0.0f, 10.0f);
    REQUIRE(bh.type == ObjectType::BlackHole);
    REQUIRE(bh.eventHorizon > 0.0f);
    REQUIRE(approxEq(bh.mass, 10.0f));
}

TEST_CASE("ParticleUtils – createComet initialises trail", "[particle][utils]") {
    Particle c = ParticleUtils::createComet(0.0f, 0.0f, 0.1f, 0.0f);
    REQUIRE(c.type == ObjectType::Comet);
    REQUIRE(c.trail != nullptr);
}

TEST_CASE("ParticleUtils – createNeutronStar is dense and fast-spinning", "[particle][utils]") {
    Particle ns = ParticleUtils::createNeutronStar(0.0f, 0.0f, 8.0f);
    REQUIRE(ns.type == ObjectType::NeutronStar);
    REQUIRE(ns.density > 1000.0f);
    REQUIRE(ns.spin > 10.0f);
}


// ============================================================
// Trail
// ============================================================

TEST_CASE("Trail – addPoint stores x and y", "[trail]") {
    Trail t;
    t.maxPoints = 5;
    t.addPoint(1.0f, 2.0f);
    REQUIRE(t.positions.size() == 2);
    REQUIRE(approxEq(t.positions[0], 1.0f));
    REQUIRE(approxEq(t.positions[1], 2.0f));
}

TEST_CASE("Trail – doesn't exceed maxPoints", "[trail]") {
    Trail t;
    t.maxPoints = 3;
    for (int i = 0; i < 10; ++i)
        t.addPoint(float(i), float(i));
    REQUIRE(t.positions.size() <= t.maxPoints * 2);
}

TEST_CASE("Trail – clear empties buffer", "[trail]") {
    Trail t;
    t.addPoint(1.0f, 2.0f);
    t.clear();
    REQUIRE(t.positions.empty());
}


// ============================================================
// Color3
// ============================================================

TEST_CASE("Color3 – lerp at t=0 gives first colour", "[color]") {
    Color3 a = {1.0f, 0.0f, 0.0f};
    Color3 b = {0.0f, 0.0f, 1.0f};
    Color3 r = Color3::lerp(a, b, 0.0f);
    REQUIRE(approxEq(r.r, 1.0f));
    REQUIRE(approxEq(r.g, 0.0f));
    REQUIRE(approxEq(r.b, 0.0f));
}

TEST_CASE("Color3 – lerp at t=1 gives second colour", "[color]") {
    Color3 a = {1.0f, 0.0f, 0.0f};
    Color3 b = {0.0f, 0.0f, 1.0f};
    Color3 r = Color3::lerp(a, b, 1.0f);
    REQUIRE(approxEq(r.r, 0.0f));
    REQUIRE(approxEq(r.b, 1.0f));
}

TEST_CASE("Color3 – lerp midpoint", "[color]") {
    Color3 a = {0.0f, 0.0f, 0.0f};
    Color3 b = {1.0f, 1.0f, 1.0f};
    Color3 r = Color3::lerp(a, b, 0.5f);
    REQUIRE(approxEq(r.r, 0.5f));
    REQUIRE(approxEq(r.g, 0.5f));
    REQUIRE(approxEq(r.b, 0.5f));
}


// ============================================================
// PhysicsWorld – object management
// ============================================================

TEST_CASE("PhysicsWorld – addObject basic", "[world]") {
    PhysicsWorld world;
    world.addObject(makeParticle(0.0f, 0.0f));
    REQUIRE(world.objects.size() == 1);
}

TEST_CASE("PhysicsWorld – addObject rejects overlapping particle", "[world]") {
    PhysicsWorld world;
    world.addObject(makeParticle(0.0f, 0.0f, 0, 0, 0.1f));
    world.addObject(makeParticle(0.05f, 0.0f, 0, 0, 0.1f)); // overlaps first
    REQUIRE(world.objects.size() == 1);
}

TEST_CASE("PhysicsWorld – addObject accepts non-overlapping particles", "[world]") {
    PhysicsWorld world;
    world.addObject(makeParticle(-0.5f, 0.0f, 0, 0, 0.05f));
    world.addObject(makeParticle( 0.5f, 0.0f, 0, 0, 0.05f));
    REQUIRE(world.objects.size() == 2);
}


// ============================================================
// PhysicsWorld – energy and momentum
// ============================================================

TEST_CASE("PhysicsWorld – totalKineticEnergy empty world", "[world][energy]") {
    PhysicsWorld world;
    REQUIRE(world.totalKineticEnergy() == 0.0f);
}

TEST_CASE("PhysicsWorld – totalKineticEnergy two particles", "[world][energy]") {
    PhysicsWorld world;
    world.addObject(makeParticle(-0.5f, 0.0f, 3.0f, 4.0f, 0.05f, 1.0f));
    world.addObject(makeParticle( 0.5f, 0.0f, 3.0f, 4.0f, 0.05f, 1.0f));
    // 2 * 0.5 * 1 * (9+16) = 25
    REQUIRE(approxEq(world.totalKineticEnergy(), 25.0f));
}

TEST_CASE("PhysicsWorld – static particles don't contribute to KE", "[world][energy]") {
    PhysicsWorld world;
    Particle p = makeParticle(0.0f, 0.0f, 10.0f, 10.0f);
    p.isStatic = true;
    world.addObject(p);
    REQUIRE(world.totalKineticEnergy() == 0.0f);
}

TEST_CASE("PhysicsWorld – totalMomentum empty world", "[world][momentum]") {
    PhysicsWorld world;
    float px, py;
    world.totalMomentum(px, py);
    REQUIRE(px == 0.0f);
    REQUIRE(py == 0.0f);
}

TEST_CASE("PhysicsWorld – totalMomentum cancels for equal and opposite", "[world][momentum]") {
    PhysicsWorld world;
    world.addObject(makeParticle(-0.5f, 0.0f,  3.0f, 0.0f, 0.05f, 2.0f));
    world.addObject(makeParticle( 0.5f, 0.0f, -2.0f, 0.0f, 0.05f, 3.0f));
    // px: 2*3 + 3*-2 = 0
    float px, py;
    world.totalMomentum(px, py);
    REQUIRE(approxEq(px, 0.0f));
    REQUIRE(approxEq(py, 0.0f));
}

TEST_CASE("PhysicsWorld – angular momentum L = x*m*vy - y*m*vx", "[world][momentum]") {
    PhysicsWorld world;
    // x=1, y=0, vy=1, mass=2 -> L = 2
    world.addObject(makeParticle(1.0f, 0.0f, 0.0f, 1.0f, 0.05f, 2.0f));
    REQUIRE(approxEq(world.totalAngularMomentum(), 2.0f));
}

TEST_CASE("PhysicsWorld – potential energy is negative between attracting masses", "[world][energy]") {
    PhysicsWorld world;
    world.addObject(makeParticle(-0.2f, 0.0f, 0, 0, 0.05f, 5.0f));
    world.addObject(makeParticle( 0.2f, 0.0f, 0, 0, 0.05f, 5.0f));
    REQUIRE(world.totalPotentialEnergy() < 0.0f);
}


// ============================================================
// PhysicsWorld – wall collisions
// ============================================================

TEST_CASE("PhysicsWorld – handleWalls clamps x inside bounds", "[world][walls]") {
    PhysicsWorld world;
    world.left = -1.0f; world.right = 1.0f;
    world.bottom = -1.0f; world.top = 1.0f;

    world.objects.push_back(makeParticle(1.5f, 0.0f, 1.0f, 0.0f, 0.05f, 1.0f));
    world.handleWalls();
    REQUIRE(world.objects[0].x <= world.right);
}

TEST_CASE("PhysicsWorld – handleWalls reverses vx on right wall", "[world][walls]") {
    PhysicsWorld world;
    world.left = -1.0f; world.right = 1.0f;
    world.bottom = -1.0f; world.top = 1.0f;

    world.objects.push_back(makeParticle(1.5f, 0.0f, 2.0f, 0.0f, 0.05f, 1.0f));
    world.handleWalls();
    REQUIRE(world.objects[0].vx < 0.0f);
}

TEST_CASE("PhysicsWorld – handleWalls reverses vy on bottom wall", "[world][walls]") {
    PhysicsWorld world;
    world.left = -1.0f; world.right = 1.0f;
    world.bottom = -1.0f; world.top = 1.0f;

    world.objects.push_back(makeParticle(0.0f, -1.5f, 0.0f, -2.0f, 0.05f, 1.0f));
    world.handleWalls();
    REQUIRE(world.objects[0].vy > 0.0f);
}

TEST_CASE("PhysicsWorld – static objects are unaffected by walls", "[world][walls]") {
    PhysicsWorld world;
    world.left = -1.0f; world.right = 1.0f;
    world.bottom = -1.0f; world.top = 1.0f;

    Particle p = makeParticle(2.0f, 2.0f, 0.0f, 0.0f, 0.05f, 1.0f);
    p.isStatic = true;
    world.objects.push_back(p);
    world.handleWalls();

    REQUIRE(approxEq(world.objects[0].x, 2.0f));
    REQUIRE(approxEq(world.objects[0].y, 2.0f));
}


// ============================================================
// PhysicsWorld – gravity and integration
// ============================================================

TEST_CASE("PhysicsWorld – downward gravity moves particle down", "[world][gravity]") {
    PhysicsWorld world;
    world.gravity = -1.0f;

    world.objects.push_back(makeParticle(0.0f, 0.0f));
    float y0 = world.objects[0].y;
    world.step(0.1f);
    REQUIRE(world.objects[0].y < y0);
}

TEST_CASE("PhysicsWorld – zero gravity, particle at rest stays put", "[world][gravity]") {
    PhysicsWorld world;
    world.gravity = 0.0f;
    world.objects.push_back(makeParticle(0.0f, 0.0f));
    world.step(1.0f);
    REQUIRE(approxEq(world.objects[0].y, 0.0f, 0.01f));
}

TEST_CASE("PhysicsWorld – particle travels in direction of velocity", "[world][integration]") {
    PhysicsWorld world;
    world.gravity = 0.0f;
    world.left = -10.0f; world.right = 10.0f;
    world.bottom = -10.0f; world.top = 10.0f;

    world.objects.push_back(makeParticle(0.0f, 0.0f, 1.0f, 0.0f, 0.05f, 1.0f));
    world.step(0.1f);
    REQUIRE(world.objects[0].x > 0.0f);
}


// ============================================================
// PhysicsWorld – N-body gravity
// ============================================================

TEST_CASE("PhysicsWorld – two masses attract each other", "[world][gravity]") {
    PhysicsWorld world;
    world.gravity = 0.0f;

    world.addObject(makeParticle(-0.3f, 0.0f, 0.0f, 0.0f, 0.03f, 50.0f));
    world.addObject(makeParticle( 0.3f, 0.0f, 0.0f, 0.0f, 0.03f, 50.0f));

    float x0 = world.objects[0].x;
    float x1 = world.objects[1].x;

    for (int i = 0; i < 20; ++i)
        world.step(0.016f);

    REQUIRE(world.objects[0].x > x0);
    REQUIRE(world.objects[1].x < x1);
}


// ============================================================
// PhysicsWorld – spatial grid
// ============================================================

TEST_CASE("PhysicsWorld – spatial grid contains all objects", "[world][grid]") {
    PhysicsWorld world;
    world.addObject(makeParticle(0.0f, 0.0f));
    world.addObject(makeParticle(0.5f, 0.5f));
    world.updateSpatialGrid();

    size_t total = 0;
    for (auto& row : world.gridCells)
        for (auto& cell : row)
            total += cell.size();

    REQUIRE(total == 2);
}


// ============================================================
// PhysicsWorld – force fields
// ============================================================

TEST_CASE("PhysicsWorld – addForceField", "[world][forcefield]") {
    PhysicsWorld world;
    ForceField f;
    f.type = ForceField::RADIAL;
    f.x = 0.0f; f.y = 0.0f;
    f.strength = 1.0f; f.radius = 0.5f;
    world.addForceField(f);
    REQUIRE(world.forceFields.size() == 1);
}

TEST_CASE("PhysicsWorld – removeForceField", "[world][forcefield]") {
    PhysicsWorld world;
    ForceField f;
    f.type = ForceField::VORTEX;
    f.x = 0.0f; f.y = 0.0f;
    f.strength = 1.0f; f.radius = 0.5f;
    world.addForceField(f);
    world.removeForceField(0);
    REQUIRE(world.forceFields.empty());
}

TEST_CASE("PhysicsWorld – clearForceFields", "[world][forcefield]") {
    PhysicsWorld world;
    for (int i = 0; i < 5; ++i) {
        ForceField f;
        f.type = ForceField::RADIAL;
        f.x = float(i) * 0.1f; f.y = 0.0f;
        f.strength = 1.0f; f.radius = 0.3f;
        world.addForceField(f);
    }
    world.clearForceFields();
    REQUIRE(world.forceFields.empty());
}

TEST_CASE("PhysicsWorld – inactive force field has no effect on velocity", "[world][forcefield]") {
    PhysicsWorld world;
    world.gravity = 0.0f;
    world.left = -10.0f; world.right = 10.0f;
    world.bottom = -10.0f; world.top = 10.0f;

    world.addObject(makeParticle(0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 1.0f));

    ForceField f;
    f.type = ForceField::RADIAL;
    f.x = 0.0f; f.y = 0.0f;
    f.strength = 100.0f; f.radius = 1.0f;
    f.active = false;
    world.addForceField(f);

    float vx_before = world.objects[0].vx;
    world.applyForceFields();
    REQUIRE(approxEq(world.objects[0].vx, vx_before));
}


// ============================================================
// PhysicsWorld – stats
// ============================================================

TEST_CASE("PhysicsWorld – stats reset", "[world][stats]") {
    PhysicsWorld world;
    world.stats.totalCollisions = 99;
    world.stats.objectsAbsorbed = 5;
    world.stats.totalEnergyLost = 12.0f;
    world.stats.reset();

    REQUIRE(world.stats.totalCollisions == 0);
    REQUIRE(world.stats.objectsAbsorbed == 0);
    REQUIRE(approxEq(world.stats.totalEnergyLost, 0.0f));
}


// ============================================================
// PhysicsWorld – air drag
// ============================================================

TEST_CASE("PhysicsWorld – air drag slows particle", "[world][drag]") {
    PhysicsWorld world;
    world.airDragCoefficient = 0.5f;

    world.objects.push_back(makeParticle(0.0f, 0.0f, 1.0f, 0.0f, 0.05f, 1.0f));

    auto speed = [&]() {
        float vx = world.objects[0].vx, vy = world.objects[0].vy;
        return std::sqrt(vx*vx + vy*vy);
    };

    float before = speed();
    world.applyAirDrag(0.1f);
    REQUIRE(speed() < before);
}

TEST_CASE("PhysicsWorld – zero drag coefficient leaves velocity unchanged", "[world][drag]") {
    PhysicsWorld world;
    world.airDragCoefficient = 0.0f;

    world.objects.push_back(makeParticle(0.0f, 0.0f, 2.0f, 2.0f, 0.05f, 1.0f));
    float vx_before = world.objects[0].vx;
    world.applyAirDrag(1.0f);
    REQUIRE(approxEq(world.objects[0].vx, vx_before));
}
