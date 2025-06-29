cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

# Include all source files recursively in src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# ImGui sources
file(GLOB IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# Add ImGui headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
)

# Create the executable
set(ALL_SOURCES ${SOURCES} ${IMGUI_SOURCES})
add_executable(PhysicsEngine ${ALL_SOURCES})

# Add include directory
target_include_directories(PhysicsEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Link required libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(PhysicsEngine PRIVATE OpenGL::GL glfw GLEW)

target_compile_definitions(PhysicsEngine PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

