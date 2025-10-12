cmake_minimum_required(VERSION 3.14)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# FetchContent for dependencies
# -----------------------------
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# GLEW
FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/nigels-com/glew.git
    GIT_TAG        glew-2.2.0
)
FetchContent_MakeAvailable(glew)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.8
)
FetchContent_MakeAvailable(imgui)

# -----------------------------
# ImGui backend sources
# -----------------------------
set(IMGUI_BACKENDS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# -----------------------------
# Source files
# -----------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# -----------------------------
# Add ImGui library
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

target_link_libraries(imgui_lib PUBLIC glfw glew)

# -----------------------------
# Create executable
# -----------------------------
add_executable(PhysicsEngine ${SOURCES})
target_include_directories(PhysicsEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(PhysicsEngine PRIVATE imgui_lib OpenGL::GL)

# -----------------------------
# Optional: Print info
# -----------------------------
message(STATUS "Dependencies loaded: GLFW, GLEW, ImGui")
