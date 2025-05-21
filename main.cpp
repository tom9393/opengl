#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/shaderClass.h"
#include "include/Camera.h"
#include "include/oldtexture.h"
#include "include/Window.h"
#include "include/Input.h"
#include "include/lighting.h"
#include "include/model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -4.0f), 
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

	/*
	OldTexture texture1 = OldTexture("/home/tom/Coding/c++/opengl/assets/container.jpg", false, GL_TEXTURE0);
	OldTexture texture2 = OldTexture("/home/tom/Coding/c++/opengl/assets/awesomeface.png", true, GL_TEXTURE1);
	OldTexture texture3 = OldTexture("/home/tom/Coding/c++/opengl/assets/cat.jpg", false, GL_TEXTURE2);
	*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, InputHandler::mouse_callback);

	Lighting lighting;

	if (!lighting.Init()) {
	    return 1;
	};

	PointLight pl;
	pl.setPosition(glm::vec3(0.0f, 2.5f, 0.0f));

	PointLight pl2;
	pl2.setPosition(glm::vec3(0.0f, 2.5f, 0.0f));

	DirLight dl;

	SpotLight sl;

	SpotLight sl2;

	Model sukhoi("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor("/home/tom/Coding/c++/opengl/assets/floor.obj");

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		InputHandler::process(window);
		Camera camera = InputHandler::getCamera();

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		float lightX = 5.0f * sin(1 * glfwGetTime());
		float lightY = 0.0f;
		float lightZ = 5.0f * cos(1 * glfwGetTime());
		glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);

		float lightX2 = 0.0f;
		float lightY2 = 0.0f;
		float lightZ2 = 5.0f * cos(1 * glfwGetTime());
		glm::vec3 lightPos2 = glm::vec3(lightX2, lightY2, lightZ2);

		pl.setPosition(lightPos);
		pl2.setPosition(lightPos2);

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

		sl.setPosition(lightPos);
		sl.setDirection(glm::vec3(0.0f));

		sl2.setPosition(lightPos2);
		sl2.setDirection(glm::vec3(0.0f, 0.0f, 1.0f));

		lighting.Enable();
		lighting.setPointlight(0, pl);
		// lighting.setPointlight(1, pl2);
		lighting.setDirlight(0, dl);
		lighting.setSpotlight(0, sl);
		lighting.setSpotlight(1, sl2);

		lighting.SetCameraLocalPos(camera.Position);

		lighting.SetView(view);
		lighting.SetProjection(projection);
	
		glm::mat4 mm = glm::mat4(1.0f);
		model = glm::translate(mm, cubePositions[0]);
		lighting.SetModel(model);

		tank.Draw(lighting);

		mm = glm::mat4(1.0f);
		model = glm::translate(mm, cubePositions[1]);
		lighting.SetModel(model);

		sukhoi.Draw(lighting);

		mm = glm::mat4(1.0f);
		model = glm::translate(mm, glm::vec3(0.0f, -1.55f, 0.0f));
		lighting.SetModel(model);

		floor.Draw(lighting);
		/*
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
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	lightShader.remove();
	glfwDestroyWindow(window);
	glfwTerminate();
	/*
	texture1.Unload();
	texture2.Unload();
	texture3.Unload();
	*/
	return 0;
}
