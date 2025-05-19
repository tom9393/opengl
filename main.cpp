#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/material.h"
#include "include/shaderClass.h"
#include "include/Camera.h"
#include "include/Texture.h"
#include "include/Window.h"
#include "include/Input.h"
#include "include/lighting.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

int main()
{
	Window windowMaker = Window();
	windowMaker.initialize(1200, 1200, "Opengl");
	GLFWwindow* window = windowMaker.getWindow();
	if (window == NULL) {
	    std::cout << "Could not create window" << std::endl;
	    return -1;
	}

	OldShader lightShader("/home/tom/Coding/c++/opengl/shaders/light.vert", "/home/tom/Coding/c++/opengl/shaders/light.frag");

	Cube cube;
	cube.init();

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

	Texture texture1 = Texture("/home/tom/Coding/c++/opengl/assets/container.jpg", false, GL_TEXTURE0);
	Texture texture2 = Texture("/home/tom/Coding/c++/opengl/assets/awesomeface.png", true, GL_TEXTURE1);
	Texture texture3 = Texture("/home/tom/Coding/c++/opengl/assets/cat.jpg", false, GL_TEXTURE2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, InputHandler::mouse_callback);

	Material material;
	material.name = "Somename";
	material.ambient = glm::vec3(1.0f, 0.5f, 0.31f);
	material.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
	material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	material.shininess = 32.0f;

	Lighting lighting;

	if (!lighting.Init()) {
	    return 1;
	};

	PointLight pl;
	pl.ambient = glm::vec3(0.05f);
	pl.diffuse = glm::vec3(0.8f);
	pl.specular = glm::vec3(1.0f);
	pl.constant = 1.0f;
	pl.linear = 0.09f;
	pl.quadratic = 0.032f;
	pl.position = glm::vec3(0.0f, 2.5f, 0.0f);

	PointLight pl2;
	pl2.ambient = glm::vec3(0.05f);
	pl2.diffuse = glm::vec3(0.8f);
	pl2.specular = glm::vec3(1.0f);
	pl2.constant = 1.0f;
	pl2.linear = 0.09f;
	pl2.quadratic = 0.032f;
	pl2.position = glm::vec3(0.0f, 2.5f, 0.0f);

	DirLight dl;
	dl.direction = glm::vec3(-0.2f, -3.0f, -2.0f);
	dl.ambient = glm::vec3(0.01f);
	dl.diffuse = glm::vec3(0.2f);
	dl.specular = glm::vec3(0.25f);

	SpotLight sl;
	sl.ambient = glm::vec3(0.0f);
	sl.specular = glm::vec3(1.0f);
	sl.diffuse = glm::vec3(1.0f);
	sl.constant = 1.0f;
	sl.quadratic = 0.032f;
	sl.linear = 0.09f;
	sl.cutOff = glm::cos(glm::radians(12.5f));
	sl.outerCutOff = glm::cos(glm::radians(17.5f));

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		InputHandler::process(window);
		Camera camera = InputHandler::getCamera();

		sl.position = camera.Position;
		sl.direction = camera.Front;

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		float lightX = 5.0f * sin(2 * glfwGetTime());
		float lightY = 0.0f;
		float lightZ = 5.0f * cos(2 * glfwGetTime());
		float lightX2 = 5.0f * cos(5 * glfwGetTime());
		glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);
		glm::vec3 lightPos2 = glm::vec3(lightZ, lightX2, lightY);

		pl.position = lightPos;
		pl2.position = lightPos2;

		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 viewMatrix = glm::lookAt(lightPos, target, up);

		glm::mat4 rotationMatrix = glm::inverse(viewMatrix);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model *= rotationMatrix;  // Apply rotation to face the center
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		lightShader.setMat4("model", model);

	        glBindVertexArray(lightCubeVAO);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		viewMatrix = glm::lookAt(lightPos2, target, up);

		rotationMatrix = glm::inverse(viewMatrix);

		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos2);
		model *= rotationMatrix;  // Apply rotation to face the center
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		lightShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		lighting.Enable();
		lighting.setPointlight(0, pl);
		lighting.setPointlight(1, pl2);
		lighting.setDirlight(0, dl);
		lighting.setSpotlight(0, sl);

		lighting.SetCameraLocalPos(camera.Position);

		lighting.SetView(view);
		lighting.SetProjection(projection);
	
		lighting.SetTexture("material.diffuse", 0);
		lighting.SetTexture("material.specular", 1);

		cube.activate();

		for (int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * glfwGetTime() * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lighting.SetModel(model);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		cube.deactivate();

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	lightShader.remove();
	glfwDestroyWindow(window);
	glfwTerminate();
	texture1.Unload();
	texture2.Unload();
	texture3.Unload();
	return 0;
}
