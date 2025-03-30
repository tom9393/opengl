#include "../Headers/Input.h"
#include "../Headers/Camera.h"

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400;
float lastY = 300;
bool firstMouse = true;

Camera_Movement InputHandler::processInput(GLFWwindow *window)
{
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
	
	return direction;
}

void InputHandler::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset, true);
}

Camera InputHandler::getCamera() {
	    return camera;
}

void InputHandler::process(GLFWwindow* window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    Camera_Movement direction = processInput(window);

    camera.ProcessKeyboard(direction, deltaTime);
}
