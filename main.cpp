#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/model.h"
#include "include/obj.h"
#include "include/Window.h"
#include "include/mainshader.h"
#include "include/path.h"
#include "include/scene.h"
#include "include/shadernode.h"
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
	Window window(SCREEN_HEIGHT, SCREEN_WIDTH, "Opengl");
	if (window.getWindow() == NULL) {
	    std::cout << "Could not create window" << std::endl;
	    return -1;
	}

	Mainshader mainshader;
	Shadowshader shadowshader;

	if (!mainshader.Init() || !shadowshader.Init())
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
	Lightmanager lightmanager;
	lightmanager.addLight(&dl);
	lightmanager.addLight(&sl);
	lightmanager.addLight(&sl2);
	lightmanager.addLight(&pl);
	lightmanager.addLight(&pl2);
	ShaderNode mainNode(mainshader);
	ShaderNode shadowNode(shadowshader);
	mainNode.addBefore(&shadowNode);

	mainshader.lightmanager = &lightmanager;
	shadowshader.lightmanager = &lightmanager;

	scene.addWindow(&window);
	scene.setRoot(&mainNode);

	std::vector<ShaderNode*> nodes;
	nodes.push_back(&mainNode);
	nodes.push_back(&shadowNode);

	for (unsigned int i = 0; i < nodes.size(); i++) {
		nodes[i]->addDrawable(&sukhoi);
		nodes[i]->addDrawable(&tank);
		nodes[i]->addDrawable(&floor);
	}

	glm::vec3 rot = glm::vec3(0.0f, glm::radians(1.0f), 0.0f);
	sukhoi.setAngularVelocity(rot);

	std::vector<glm::vec3> thingeys;

	for (unsigned int i = 0; i < 20; i++)
	{
		float x = i;
		float y = i;
		thingeys.push_back(glm::vec3(sin(i) * x, 2.0f, cos(i) * y));
	}

	Path mypath(sukhoi.location);
	mypath.setPath(thingeys);

	scene.createBatches();

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		lightPos = glm::vec3(5.0f * sin(glfwGetTime()), -0.25f, 5.0f * cos(glfwGetTime()));
		lightPos2 = glm::vec3(0.0f, 3.0f, -1.0f * 1.0f * cos(glfwGetTime()));
		sl.setPosition(lightPos);
		sl2.setPosition(lightPos2);
		sl.setDirection(sl.getDirectionToTarget(sl.getMatrix()));
		sl2.setDirection(sl2.getDirectionToTarget(sl2.getMatrix()));

		float dt = 0.2f;
		mypath.update(dt, testfun);

		scene.draw();
	}

	glfwDestroyWindow(window.getWindow());
	glfwTerminate();
	return 0;
}
