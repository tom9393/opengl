cmake_minimum_required(VERSION 4.1.2)

project(opengl)

file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
    platform/Linux/*.cpp
    platform/OpenGL/*.cpp
    include/**/*.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${ENGINE_SOURCES})

# === Third-party libs ===
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/yaml-cpp)

target_include_directories(${PROJECT_NAME}
	PUBLIC include
	PUBLIC include/Scene
	PUBLIC include/Panels
	PUBLIC include/Render
	PUBLIC platform/Linux
	PUBLIC platform/OpenGL
	
	PUBLIC lib/glad
	PUBLIC lib/imgui
	PUBLIC lib/imgui/backends
	PUBLIC lib/imguizmo
	PUBLIC lib/entt

	PUBLIC lib/glfw/include
)

target_link_libraries(${PROJECT_NAME} GL dl glfw glm assimp yaml-cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch.cpp>)
