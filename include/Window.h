#ifndef WINDOW_H
#define WINDOW_H

#include "Camera.h"
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(unsigned int width, unsigned int height, const char* title) : width(width), height(height), title(title), camera_(new Camera()), lastframe(0.0f), deltatime(0.0f) {
        // Initialize GLFW
        glfwInit();

        // Tell GLFW what version of OpenGL we are using 
        // In this case we are using OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        // Tell GLFW we are using the CORE profile
        // So that means we only have the modern functions
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
        window_ = glfwCreateWindow(width, height, title, NULL, NULL);
        // Error check if the window fails to create
        if (window_ == NULL)
        {
                glfwTerminate();
                return;
        }
        // Introduce the window into the current context
        glfwMakeContextCurrent(window_);

        glfwSetWindowUserPointer(window_, this);
        glfwSetCursorPosCallback(window_, mouse_callback_);

        //Load GLAD so it configures OpenGL
        gladLoadGL();
        // Specify the viewport of OpenGL in the Window
        // In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
        glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    };
    GLFWwindow* getWindow();
    Camera* getCamera();
    void updateDelta();
    static void key_callback_(GLFWwindow* window_);
    static void mouse_callback_(GLFWwindow* window_, double xpos, double ypos);

private:
    unsigned int height;
    unsigned int width;
    const char* title;
    GLFWwindow* window_;
    Camera* camera_;
    float lastframe;
    float deltatime;
};

#endif
