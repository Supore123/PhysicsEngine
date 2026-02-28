# PhysicsEngine

A 2D physics engine built in C++ with an OpenGL/ImGui frontend. Started as a sandbox for testing physics systems and has grown to include n-body gravity, collision detection, force fields, and a basic celestial object simulation. Still a work in progress — expect rough edges.

---

## What it does

- N-body gravitational attraction between objects
- Rigid-body collision detection and response (spatial grid broadphase + impulse resolution)
- Continuous collision detection to prevent tunnelling at high speeds
- Force fields: radial, vortex, and directional
- Air drag
- Celestial object types: stars, planets, black holes, neutron stars, comets, asteroids, gas giants
- Black hole absorption, supernova events, tidal disruption
- Comet trails
- Temperature-based star colouring
- Gravity field visualisation (arrow grid, colour-mapped by field strength)
- ImGui control panel: add/remove objects, tweak physics parameters, load preset scenarios

---

## Project structure

```
PhysicsEngine/
├── inc/            # Header files
├── src/            # Source files
├── tests/          # Unit tests (Catch2)
├── third_party/    # External dependencies (gitignored)
└── CMakeLists.txt
```

---

## Dependencies

**System packages** — install these first:

```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev libglu1-mesa-dev libglew-dev
```

**Fetched automatically by CMake:**
- GLFW 3.3.8
- ImGui v1.89.8

**For tests only:**
- Catch2 (single header, see Testing section below)

---

## Building

```bash
git clone https://github.com/Supore123/PhysicsEngine.git
cd PhysicsEngine
cmake -S . -B build
cmake --build build
./build/PhysicsEngine
```

---

## Controls

| Input | Action |
|---|---|
| `M` | Spawn object at cursor position |
| `Backspace` | Remove last non-static object |
| `P` | Pause / unpause |
| `Escape` | Quit |

Mouse input is handled through the ImGui panel — use it to add specific object types, adjust gravity, load scenarios, etc.

---

## Testing

Tests use [Catch2](https://github.com/catchorg/Catch2) (v2.x, single-header). You need to download the header once:

```bash
curl -L https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp \
     -o tests/catch.hpp
```

Then build and run:

```bash
cmake -S . -B build
cmake --build build
./build/PhysicsTests
```

Or via CTest:

```bash
cd build && ctest --output-on-failure
```

The tests cover: particle kinematics, energy/momentum diagnostics, wall collisions, n-body attraction, spatial grid, force fields, air drag, and all the `ParticleUtils` factory functions. See `tests/test_physics.cpp` for the full list.

> `catch.hpp` is excluded from version control (.gitignore) — you'll need to re-download it after a fresh clone.

---

## Known issues / TODO

- [ ] Tunnelling fix (Issue #4) — CCD substep count is a rough heuristic, not robust for very fast small objects
- [ ] Numerical instability with degenerate shapes (Issue #5)
- [ ] Multi-contact solver needs proper iterative resolution rather than single-pass (Issue #6)
- [ ] No broad-phase culling for the gravity field renderer — gets slow with many objects
- [ ] Trails aren't rendered yet despite the Trail struct existing in `particle.hpp`
