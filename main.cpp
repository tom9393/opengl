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

	stbi_set_flip_vertically_on_load(true);

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

	Cube cube;
	cube.init();
	Cube cube2;
	cube2.init();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		InputHandler::process(window);
		Camera camera = InputHandler::getCamera();

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		glm::vec3 lightPos = glm::vec3(5.0f * sin(glfwGetTime()), 0.0f, 5.0f * cos(glfwGetTime()));
		glm::vec3 lightPos2 = glm::vec3(0.0f, 0.0f, 5.0f * cos(glfwGetTime()));

		pl.setPosition(lightPos);
		pl2.setPosition(lightPos2);

		cube.setLocation(lightPos);
		cube.setRotationTarget(glm::vec3(0.0f));
		cube.draw(lightShader);

		cube2.setLocation(lightPos2);
		cube2.setRotationTarget(glm::vec3(0.0f));
		cube2.draw(lightShader);

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
		glm::mat4 model = glm::translate(mm, glm::vec3(0.0f));
		lighting.SetModel(model);

		tank.Draw(lighting);

		mm = glm::mat4(1.0f);
		model = glm::translate(mm, glm::vec3(2.0f, 5.0f, -4.0f));
		lighting.SetModel(model);

		sukhoi.Draw(lighting);

		mm = glm::mat4(1.0f);
		model = glm::translate(mm, glm::vec3(0.0f, -1.55f, 0.0f));
		lighting.SetModel(model);

		floor.Draw(lighting);

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	lightShader.remove();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
