cmake_minimum_required(VERSION 4.1.2)

project(opengl)

add_executable(${PROJECT_NAME} main.cpp)

# === Third-party libs ===
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/imgui)
add_subdirectory(lib/imguizmo)
add_subdirectory(lib/yaml-cpp)

target_include_directories(${PROJECT_NAME}
	PUBLIC include
	PUBLIC include/Scene
	PUBLIC include/Panels
	PUBLIC include/Render
	PUBLIC platform/Linux
	PUBLIC platform/OpenGL
	
	PUBLIC lib/glad

	PUBLIC lib/glfw/include
)

target_link_directories(${PROJECT_NAME}
	PRIVATE src
	PRIVATE include/Scene
	PRIVATE include/Panels
	PRIVATE include/Render
	PRIVATE platform/Linux
	PRIVATE platform/OpenGL

	PRIVATE lib/glad

	PRIVATE lib/glfw/src
)

target_link_libraries(${PROJECT_NAME} GL dl glfw assimp yaml-cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch.cpp>)
