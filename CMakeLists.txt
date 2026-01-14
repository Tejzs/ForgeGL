cmake_minimum_required(VERSION 3.5)
project(ForgeGL, VERSION 0.0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file(GLOB source_files CONFIGURE_DEPENDS
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/core/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/core/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/editor/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/engine/editor/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Include directories
include_directories(
    lib/imgui
    lib/imgui/backends
)

# ImGui source files
set(IMGUI_SRC
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)


# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/glfw-3.4")
find_package(OpenGL REQUIRED)


# GLAD
add_subdirectory("lib/glad/")


# GLM
add_subdirectory("lib/glm")


# STB_IMAGE
add_subdirectory("lib/stb_image/")


# ASSIMP
add_subdirectory("lib/assimp/")
set_target_properties(assimp PROPERTIES VERSION "5.3.0" SOVERSION "5")

# ReactPhyscis3d
list(APPEND CMAKE_PREFIX_PATH "/usr/local/lib/cmake/ReactPhysics3D")
find_package(ReactPhysics3D REQUIRED)

# Entt
add_subdirectory(lib/entt)

# Json
add_subdirectory(lib/json)

# Include directories
include_directories(
    lib/imgui
    lib/imgui/backends
    lib/tracy/public
)


add_executable(ForgeGL src/main.cpp ${source_files} ${IMGUI_SRC})
target_link_libraries(ForgeGL glfw OpenGL::GL glad glm stb_image assimp ReactPhysics3D::ReactPhysics3D EnTT::EnTT nlohmann_json)