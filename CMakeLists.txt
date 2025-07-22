cmake_minimum_required(VERSION 3.17)

project(opengl.exe)

file(GLOB LIB_SOURCES "lib/*.c" "lib/*.cpp" "lib/*.h")
file(GLOB SRC_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${LIB_SOURCES} ${SRC_SOURCES})

target_link_libraries(${PROJECT_NAME} GL dl glfw assimp)
