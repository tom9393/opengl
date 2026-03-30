cmake_minimum_required(VERSION 4.3.0)

project(opengl)

file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
    platform/*.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${ENGINE_SOURCES})

# === Disable Bullet examples, extras, and demos ===
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Disable bullet demos" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "Disable bullet extras" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Disable bullet unit tests" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)

# So far no support for lua 5.5
find_package(Lua 5.4 EXACT REQUIRED)

# === Third-party libs ===
add_subdirectory(lib/bullet)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/yaml-cpp)
add_subdirectory(lib/ktx)

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
	PUBLIC src
	PUBLIC src/Scene
	PUBLIC src/Panels
	PUBLIC src/Render
	PUBLIC platform/Linux
	PUBLIC platform/OpenGL
	PUBLIC ${LUA_INCLUDE_DIR} # Must be included before lua or otherwise it tries to use 5.5
	
	PUBLIC lib/entt # Entt is a single header library
	PUBLIC lib/sol2 # Same as above

	PUBLIC lib/bullet/src # Bullet expects this to work

	PUBLIC lib/ktx/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GL
    dl
    glfw
    stb
    imgui
    glad
    glm
    imguizmo
    assimp
    yaml-cpp
    BulletDynamics
    BulletCollision
    LinearMath
    Bullet3Common
    ktx
    ${LUA_LIBRARIES}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
