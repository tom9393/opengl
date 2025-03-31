#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "Headers/shaderClass.h"
#include "Headers/VAO.h"
#include "Headers/VBO.h"
#include "Headers/Camera.h"
#include "Headers/Texture.h"
#include "Headers/Window.h"
#include "Headers/Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float vertices[] = {
    // Positions        // Normals         // Texture Coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
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
	windowMaker.initialize(1200, 1200, "Opengl");
	GLFWwindow* window = windowMaker.getWindow();
	if (window == NULL) {
	    std::cout << "Could not create window" << std::endl;
	    return -1;
	}

	// Generates Shader object using shaders defualt.vert and default.frag for object
	Shader shaderProgram("/home/tom/Coding/c++/opengl/Shaders/default.vert", "/home/tom/Coding/c++/opengl/Shaders/default.frag");
	
	// Generate shader for the light
	Shader shaderProgram2("/home/tom/Coding/c++/opengl/Shaders/light.vert", "/home/tom/Coding/c++/opengl/Shaders/light.frag");

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));


	// Links VBO to VAO
	VAO1.LinkVBO(VBO1);
	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();

	VAO VAO2;

	VAO2.Bind();
	VAO2.LinkVBO(VBO1);
	VAO2.Unbind();

	VBO1.Unbind();

	float verts[] = {
	    // Base (square) - Two triangles
	    -0.5f,  0.0f, -0.5f,  // Bottom-left
	     0.5f,  0.0f, -0.5f,  // Bottom-right
	     0.5f,  0.0f,  0.5f,  // Top-right

	    -0.5f,  0.0f, -0.5f,  // Bottom-left
	     0.5f,  0.0f,  0.5f,  // Top-right
	    -0.5f,  0.0f,  0.5f,  // Top-left

	    // Side 1
	    -0.5f,  0.0f, -0.5f,  // Bottom-left
	     0.5f,  0.0f, -0.5f,  // Bottom-right
	     0.0f,  0.7f,  0.0f,  // Top

	    // Side 2
	     0.5f,  0.0f, -0.5f,  // Bottom-left
	     0.5f,  0.0f,  0.5f,  // Bottom-right
	     0.0f,  0.7f,  0.0f,  // Top

	    // Side 3
	     0.5f,  0.0f,  0.5f,  // Bottom-left
	    -0.5f,  0.0f,  0.5f,  // Bottom-right
	     0.0f,  0.7f,  0.0f,  // Top

	    // Side 4
	    -0.5f,  0.0f,  0.5f,  // Bottom-left
	    -0.5f,  0.0f, -0.5f,  // Bottom-right
	     0.0f,  0.7f,  0.0f   // Top
	};


	
	unsigned int lightCubeVAO, vbo2;
	glGenVertexArrays(1, &lightCubeVAO);
	glGenBuffers(1, &vbo2);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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

		shaderProgram.use();

		shaderProgram.setMat4("view", view);
		shaderProgram.setMat4("projection", projection);

		float lightX = 5.0f * sin(2 * glfwGetTime());
		float lightY = 0.0f;
		float lightZ = 5.0f * cos(2 * glfwGetTime());
		glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);

		glm::vec3 lightColor;
		// lightColor.x = sin(glfwGetTime() * 2.0f);
		// lightColor.y = sin(glfwGetTime() * 0.7f);
		// lightColor.z = sin(glfwGetTime() * 1.3f);
		lightColor.x = 1.0f;
		lightColor.y = 1.0f;
		lightColor.z = 1.0f;
		  
		glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); 
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

		shaderProgram.setVec3("light.ambient",  ambientColor);
		shaderProgram.setVec3("light.diffuse",  diffuseColor);
		shaderProgram.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		shaderProgram.setFloat("light.constant",  1.0f);
		shaderProgram.setFloat("light.linear",    0.09f);
		shaderProgram.setFloat("light.quadratic", 0.032f);

		shaderProgram.setVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
		shaderProgram.setVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
		shaderProgram.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		shaderProgram.setFloat("material.shininess", 32.0f);
		shaderProgram.setVec3("light.position", lightPos);
		shaderProgram.setVec3("viewPos", camera.Position);

		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);

		shaderProgram.setInt("texture1", 0);
		shaderProgram.setInt("texture2", 1);

		VAO1.Bind();

		for (int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * glfwGetTime() * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			shaderProgram.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		VAO1.Unbind();

		shaderProgram2.use();
		shaderProgram2.setMat4("projection", projection);
		shaderProgram2.setMat4("view", view);

		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 viewMatrix = glm::lookAt(lightPos, target, up);

		glm::mat4 rotationMatrix = glm::inverse(viewMatrix);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model *= rotationMatrix;  // Apply rotation to face the center
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		shaderProgram2.setMat4("model", model);

		// VAO2.Bind();

	        glBindVertexArray(lightCubeVAO);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		// VAO2.Unbind();

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Delete all the objects we've created
	VAO1.Delete();
	VAO2.Delete();
	VBO1.Delete();
	shaderProgram.remove();
	shaderProgram2.remove();
	glfwDestroyWindow(window);
	glfwTerminate();
	texture1.Unload();
	texture2.Unload();
	texture3.Unload();
	return 0;
}
