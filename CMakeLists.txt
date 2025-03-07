cmake_minimum_required(VERSION 3.10)

project(GravitySimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})

# Fetch glad automatically
include(FetchContent)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36  # Change this tag if desired
)
FetchContent_MakeAvailable(glad)

# List your source files
set(SOURCES
    main.cpp
    Shader.cpp
    Simulation.cpp
    CelestialBody.cpp
    SpacetimeGrid.cpp
)

set(HEADERS
    Shader.h
    Simulation.h
    CelestialBody.h
    SpacetimeGrid.h
)

# Define the executable
add_executable(gravity_sim ${SOURCES} ${HEADERS})

# Link libraries: OpenGL, GLFW (using pkg-config variables), and glad
target_link_libraries(gravity_sim ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} glad)

# Copy shader files to build directory
configure_file(${CMAKE_SOURCE_DIR}/grid_vertex_shader.glsl ${CMAKE_BINARY_DIR}/grid_vertex_shader.glsl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/grid_fragment_shader.glsl ${CMAKE_BINARY_DIR}/grid_fragment_shader.glsl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/body_vertex_shader.glsl ${CMAKE_BINARY_DIR}/body_vertex_shader.glsl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/body_fragment_shader.glsl ${CMAKE_BINARY_DIR}/body_fragment_shader.glsl COPYONLY)