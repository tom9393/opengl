#ifndef INPUT_H
#define INPUT_H

#include "Camera.h"

class InputHandler
{
public:
    static Camera_Movement processInput(GLFWwindow* window);
    static void process(GLFWwindow* window);
    static Camera getCamera();
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
};

#endif
