#include "../Headers/Window.h"

void Window::initialize(unsigned int height, unsigned int width, const char* title)
    {
        this->height = height;
        this->width = width;
        this->title = title;

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
	this->window = glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		glfwTerminate();
                return;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(this->window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, this->width, this->height);

    }

GLFWwindow* Window::getWindow() {
    return this->window;
}
