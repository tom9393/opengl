cmake_minimum_required(VERSION 3.17)

project(opengl.exe)

add_executable(${PROJECT_NAME} main.cpp glad.c Src/shaderClass.cpp Src/EBO.cpp Src/VAO.cpp Src/VBO.cpp Src/Camera.cpp Src/Texture.cpp Src/stb_image.cpp Src/Window.cpp Src/Input.cpp)

target_link_libraries(${PROJECT_NAME} GL dl glfw)
