cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

# Include all source files recursively in src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Create the executable
add_executable(PhysicsEngine ${SOURCES})

# Add include directory
target_include_directories(PhysicsEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Link required libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(PhysicsEngine PRIVATE OpenGL::GL glfw)
