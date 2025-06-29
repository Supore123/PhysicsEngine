cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

add_executable(PhysicsEngine src/main.cpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(PhysicsEngine PRIVATE OpenGL::GL glfw)
