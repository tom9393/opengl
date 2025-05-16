#ifndef WINDOW_H
#define WINDOW_H

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    void initialize(unsigned int height, unsigned int width, const char* title);
    GLFWwindow* getWindow();

private:
    unsigned int height;
    unsigned int width;
    const char* title;
    GLFWwindow* window;
};

#endif
