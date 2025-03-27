#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "Headers/shaderClass.h"
#include "Headers/VAO.h"
#include "Headers/VBO.h"
#include "Headers/EBO.h"
#include "Headers/Camera.h"
#include "Headers/Texture.h"
#include "Headers/Window.h"
#include "Headers/Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

float randomFloat()
{
    return (float)(rand()) / (float)(RAND_MAX);
}

int main()
{
	Window windowMaker = Window();
	windowMaker.initialize(800, 800, "Opengl");
	GLFWwindow* window = windowMaker.getWindow();
	if (window == NULL) {
	    std::cout << "Could not create window" << std::endl;
	    return -1;
	}

	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram("/home/tom/Coding/c++/opengl/Shaders/default.vert", "/home/tom/Coding/c++/opengl/Shaders/default.frag");
    
	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram2("/home/tom/Coding/c++/opengl/Shaders/default.vert", "/home/tom/Coding/c++/opengl/Shaders/colors.fs");
	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram3("/home/tom/Coding/c++/opengl/Shaders/default.vert", "/home/tom/Coding/c++/opengl/Shaders/light.fs");

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices, sizeof(indices));

	// Links VBO to VAO
	VAO1.LinkVBO(VBO1);
	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	stbi_set_flip_vertically_on_load(true);

	Texture texture1 = Texture("/home/tom/Coding/c++/opengl/container.jpg", false, GL_TEXTURE0);
	Texture texture2 = Texture("/home/tom/Coding/c++/opengl/awesomeface.png", true, GL_TEXTURE1);
	Texture texture3 = Texture("/home/tom/Coding/c++/opengl/cat.jpg", false, GL_TEXTURE2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Tell OpenGL which Shader Program we want to use
	shaderProgram.use();

	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, InputHandler::mouse_callback);

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		InputHandler::process(window);

		Camera camera = InputHandler::getCamera();

		// Move around
		glm::mat4 view = camera.getViewMatrix();

		// Projection
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		// shaderProgram.setMat4("view", view);
		// shaderProgram.setMat4("projection", projection);
		shaderProgram2.use();

		shaderProgram2.setMat4("projection", projection);
		shaderProgram2.setMat4("view", view);

		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);

		// shaderProgram.setInt("texture1", 0);
		// shaderProgram.setInt("texture2", 1);

		shaderProgram2.setVec3("objectColor", glm::vec3(1.0f, 0.5f, (float)sin(glfwGetTime())));
		shaderProgram2.setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

		for (int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * glfwGetTime() * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			// shaderProgram.setMat4("model", model);
			shaderProgram2.setMat4("model", model);

			VAO1.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		shaderProgram3.use();
	        shaderProgram3.setMat4("projection", projection);
		shaderProgram3.setMat4("view", view);

		shaderProgram.setInt("texture1", 2);
		shaderProgram.setInt("texture2", 2);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, sin(glfwGetTime()) * 10.0f, 0.0f));
		shaderProgram3.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		// glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw using wireframe only
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Draw primitives, number of indices, datatype of indices, index of indices
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		VAO1.Unbind();

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Delete all the objects we've created
	VAO1.Delete();
	VBO1.Delete();
	shaderProgram.remove();
	glfwDestroyWindow(window);
	glfwTerminate();
	texture1.Unload();
	texture2.Unload();
	texture3.Unload();
	return 0;
}
