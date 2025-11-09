# PhysicsEngine

A lightweight C++ physics engine setup for prototyping and testing basic physics systems.  
Designed as a foundation to build up from — expect continuous updates and improvements.

---

## Features & Goals

- Simple core for **rigid-body dynamics**, collision detection / response, and constraint solving  
- Modularity so you can extend or swap subsystems (e.g. integrate better integrators, collision algorithms, etc.)  
- Intended as a sandbox / learning tool rather than a production-grade engine  
- CMake-based build for cross-platform flexibility  

---

## Repository Layout


- **inc/** — declarations of physics classes, utilities, etc.  
- **src/** — source (.cpp) files implementing the core logic  
- **third_party/** — helper libraries, external code you rely on  
- **CMakeLists.txt** — build configuration (targets, includes, dependencies)  

---

## Getting Started (Build & Run)

### Prerequisites

- A modern C++ compiler (supporting at least C++17)  
- CMake (≥ 3.10 suggested)  
- rendering / UI library if you integrate a frontend (e.g. ImGui, OpenGL) is required for handling the outport viewing

### Build Instructions

```bash
git clone https://github.com/Supore123/PhysicsEngine.git
cd PhysicsEngine
mkdir build
cd build
cmake ..
cmake --build .
