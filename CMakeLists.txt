cmake_minimum_required(VERSION 3.17)

project(opengl.exe)

file(GLOB LIB_SOURCES "lib/*.c" "lib/*.cpp" "lib/*.h" "lib/*.hpp" "platform/Linux/*.h" "platform/OpenGL/*.h")
file(GLOB SRC_SOURCES "src/*.cpp" "include/Scene/*.cpp" "include/Panels/*.cpp" "include/Render/*.cpp" "platform/Linux/*.cpp" "platform/OpenGL/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${LIB_SOURCES} ${SRC_SOURCES})

target_link_libraries(${PROJECT_NAME} GL dl glfw assimp yaml-cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch.cpp>)
