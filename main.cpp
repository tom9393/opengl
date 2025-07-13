#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/cubeshader.h"
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
	Cubeshader cubeshader;

	if (!mainshader.Init() || !shadowshader.Init() || !cubeshader.Init())
	{
	    return 1;
	};

	stbi_set_flip_vertically_on_load(true);

	DirLight dl;
	PointLight pl, pl2;
	SpotLight sl, sl2;

	Model sukhoi_m("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank_m("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor_m("/home/tom/Coding/c++/opengl/assets/floor.obj");
	Obj sukhoi(sukhoi_m, glm::vec3(0.0f, 2.0f, 0.0f));
	Obj tank(tank_m, glm::vec3(0.0f));
	Obj floor(floor_m, glm::vec3(0.0f, -1.55f, 0.0f));
	Cube cube1, cube2;
	cube1.setLight(&sl);
	cube2.setLight(&sl2);

	std::vector<IDrawable*> objs;
	objs.push_back(&sukhoi);
	objs.push_back(&tank);
	objs.push_back(&floor);

	std::vector<ILight*> lights;
	lights.push_back(&dl);
	lights.push_back(&sl);
	lights.push_back(&sl2);
	lights.push_back(&pl);
	lights.push_back(&pl2);

	mainshader.setView((IView*)window.getCamera());
	shadowshader.setView((IView*)window.getCamera());
	cubeshader.setView((IView*)window.getCamera());

	Scene scene;
	Lightmanager lightmanager;
	lightmanager.addLight(&lights);
	ShaderNode mainNode(mainshader);
	ShaderNode shadowNode(shadowshader);
	ShaderNode cubeNode(cubeshader);
	mainNode.addAfter(&cubeNode);
	mainNode.addBefore(&shadowNode);

	mainshader.lightmanager = &lightmanager;
	shadowshader.lightmanager = &lightmanager;

	scene.setRoot(&mainNode);

	shadowNode.addDrawable(&objs);
	mainNode.addDrawable(&objs);
	cubeNode.addDrawable(&cube1);
	cubeNode.addDrawable(&cube2);

	scene.createBatches();

	std::vector<glm::vec3> thingeys;
	for (unsigned int i = 0; i < 20; i++)
	{
		thingeys.push_back(glm::vec3(sin(i) * i, 2.0f, cos(i) * i));
	}

	Path mypath(sukhoi.getTransform());
	mypath.setPath(thingeys);


	while (!glfwWindowShouldClose(window.getWindow()))
	{
		window.predraw();
		float x = sin(glfwGetTime()) * 5.0f;
		float y = cos(glfwGetTime()) * 5.0f;
		glm::vec3 pos1 = glm::vec3(x * -1.0f, -0.25f, y);
		glm::vec3 pos2 = glm::vec3(x, 3.0f, y);
		cube1.setLocation(&pos1);
		cube2.setLocation(&pos2);

		mypath.update(0.2f, testfun);

		scene.draw();
		window.postdraw();
	}

	glfwDestroyWindow(window.getWindow());
	glfwTerminate();
	return 0;
}
