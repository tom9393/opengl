#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/model.h"
#include "include/obj.h"
#include "include/Window.h"
#include "include/Input.h"
#include "include/lighting.h"
#include "include/path.h"
#include "include/scene.h"
#include "include/shadowshader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 1600;

int testfun(int n) {
	std::cout << "The loop has been completed" << std::endl;
	std::cout << n << std::endl;
	return 0;
};

int main()
{
	Window windowMaker = Window();
	windowMaker.initialize(SCREEN_HEIGHT, SCREEN_WIDTH, "Opengl");
	GLFWwindow* window = windowMaker.getWindow();
	if (window == NULL) {
	    std::cout << "Could not create window" << std::endl;
	    return -1;
	}

	// stbi_set_flip_vertically_on_load(true);

	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, InputHandler::mouse_callback);

	Lighting lighting;
	Shadowshader shadowshader;

	if (!lighting.Init() || !shadowshader.Init())
	{
	    return 1;
	};

	glm::vec3 asdf = glm::vec3(0.0f, -500.0f, 0.0f);

	PointLight pl, pl2;
	pl.setPosition(asdf);
	pl2.setPosition(asdf);

	DirLight dl;
	SpotLight sl, sl2;

	Model sukhoi_m("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank_m("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor_m("/home/tom/Coding/c++/opengl/assets/floor.obj");

	Obj sukhoi(sukhoi_m, glm::vec3(0.0f, 2.0f, 0.0f));
	Obj tank(tank_m, glm::vec3(0.0f));
	Obj floor(floor_m, glm::vec3(0.0f, -1.55f, 0.0f));

	glm::vec3 lightPos, lightPos2;

	Scene scene;
	scene.addWindow(window);
	scene.addShader(&shadowshader);
	scene.addShader(&lighting);

	scene.addLight(&dl);
	scene.addLight(&sl);
	scene.addLight(&sl2);
	scene.addLight(&pl);
	scene.addLight(&pl2);

	scene.addDrawable(&sukhoi);
	scene.addDrawable(&tank);
	scene.addDrawable(&floor);
	glm::vec3 rot = glm::vec3(0.0f, glm::radians(1.0f), 0.0f);
	sukhoi.setAngularVelocity(rot);

	Path mypath(sukhoi.location);
	mypath.setPath({
		glm::vec3(0, 2, 0),
		glm::vec3(5, 2, 5),
		glm::vec3(-5, 2, 5),
		glm::vec3(-5, 5, 10),
		glm::vec3(5, 2, 5),
	});

	while (!glfwWindowShouldClose(window))
	{
		lightPos = glm::vec3(5.0f * sin(glfwGetTime()), -0.25f, 5.0f * cos(glfwGetTime()));
		lightPos2 = glm::vec3(0.0f, 3.0f, -1.0f * 1.0f * cos(glfwGetTime()));
		sl.setPosition(lightPos);
		sl2.setPosition(lightPos2);
		sl.setDirection(sl.getDirectionToTarget(sl.getMatrix()));
		sl2.setDirection(sl2.getDirectionToTarget(sl2.getMatrix()));

		// glm::vec3 velocity = glm::vec3(sin(glfwGetTime()) / 100, 0.0f, 0.0f);
		// sukhoi.setVelocity(velocity);
		// sukhoi.update(1);
		float dt = 0.1f;
		mypath.update(dt, testfun);

		scene.draw();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
