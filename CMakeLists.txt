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
add_subdirectory(lib/bullet)

add_library(imgui
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(glad lib/glad/glad.c)
add_library(stb lib/stb/stb_image.cpp)
add_library(imguizmo lib/imguizmo/ImGuizmo.cpp)

target_link_libraries(imguizmo imgui)
target_link_libraries(imgui glfw)

target_include_directories(imgui
    PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	lib/imgui
)
target_include_directories(glad
    PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)
target_include_directories(stb
    PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)
target_include_directories(imguizmo
    PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_include_directories(${PROJECT_NAME}
	PUBLIC include
	PUBLIC include/Scene
	PUBLIC include/Panels
	PUBLIC include/Render
	PUBLIC platform/Linux
	PUBLIC platform/OpenGL
	
	PUBLIC lib/entt
)

target_link_libraries(${PROJECT_NAME} GL dl glfw stb imgui glad glm imguizmo assimp yaml-cpp BulletCollision BulletDynamics LinearMath)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch.cpp>)
