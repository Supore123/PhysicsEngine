cmake_minimum_required(VERSION 3.14)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# System dependencies
# -----------------------------
# Threads (for GLFW)
find_package(Threads REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)
if (NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL not found. Install libgl1-mesa-dev and libglu1-mesa-dev.")
endif()

# GLEW (system library)
find_package(GLEW REQUIRED)
if (NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW not found. Install libglew-dev.")
endif()

# -----------------------------
# FetchContent dependencies
# -----------------------------
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable building docs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable building tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples")
FetchContent_MakeAvailable(glfw)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.8
)
FetchContent_MakeAvailable(imgui)

# -----------------------------
# ImGui sources
# -----------------------------
set(IMGUI_BACKENDS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# -----------------------------
# Project sources
# -----------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# -----------------------------
# ImGui library
# -----------------------------
add_library(imgui_lib
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS}
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui_lib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui_lib PUBLIC glfw GLEW::GLEW Threads::Threads)

# -----------------------------
# PhysicsEngine executable
# -----------------------------
add_executable(PhysicsEngine ${SOURCES})
target_include_directories(PhysicsEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(PhysicsEngine PRIVATE imgui_lib ${OPENGL_LIBRARIES} Threads::Threads)

# -----------------------------
# Optional info
# -----------------------------
message(STATUS "Dependencies loaded: GLFW, ImGui, OpenGL, GLEW")
