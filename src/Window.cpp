#include "../include/Window.h"

float lastX = 400;
float lastY = 300;
bool firstMouse = true;
bool on = false;

bool Window::imgui()
{
    return imgui_enabled;
}

GLFWwindow* Window::getWindow()
{
return window_;
}

Camera* Window::getCamera()
{
return camera_;
}

void Window::predraw()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::postdraw()
{
    glfwSwapBuffers(window_);
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateDelta();
}

void Window::updateDelta()
{
    float currentFrame = glfwGetTime();
    deltatime = currentFrame - lastframe;
    lastframe = currentFrame;
    Window::key_callback_(window_);
}

void Window::key_callback_(GLFWwindow* window)
{
    Window *windowManager = static_cast<Window*>(glfwGetWindowUserPointer(window));
    Camera *camera = windowManager->camera_;

    static bool t_pressed_last_frame = false;

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !t_pressed_last_frame)
    {
	windowManager->imgui_enabled = !windowManager->imgui_enabled;
	if (windowManager->imgui_enabled)
	{
	    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else
	{
	    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
    }

    t_pressed_last_frame = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;

    if (windowManager->imgui_enabled) return;

    Camera_Movement direction;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		direction = FORWARD;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		direction = BACKWARD;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		direction = LEFT;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		direction = RIGHT;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
		direction = UP;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) 
		direction = DOWN;
	
    camera->ProcessKeyboard(direction, windowManager->deltatime);
}

void Window::mouse_callback_(GLFWwindow* window, double xpos, double ypos)
{
    Window *windowManager = static_cast<Window*>(glfwGetWindowUserPointer(window));
    Camera* camera = windowManager->camera_;

    float x = static_cast<float>(xpos);
    float y = static_cast<float>(ypos);

    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top

    lastX = x;
    lastY = y;

    if (windowManager->imgui_enabled) return; // ignore mouse input when using ImGui

    camera->ProcessMouseMovement(xoffset, yoffset, true);
}
