#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/cubeshader.h"
#include "include/lightmanager.h"
#include "include/model.h"
#include "include/obj.h"
#include "include/Window.h"
#include "include/mainshader.h"
#include "include/path.h"
#include "include/shadowshader.h"
#include "include/preprocessor.h"

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

	mainshader.setView((IView*)window.getCamera());
	shadowshader.setView((IView*)window.getCamera());
	cubeshader.setView((IView*)window.getCamera());

	if (!mainshader.Init() || !shadowshader.Init() || !cubeshader.Init())
	{
	    return 1;
	};

	stbi_set_flip_vertically_on_load(true);

	DirectionalLight dl;
	PointLight pl, pl2;
	SpotLight sl, sl2;

	Model sukhoi_m("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank_m("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor_m("/home/tom/Coding/c++/opengl/assets/floor.obj");
	Obj sukhoi(sukhoi_m, glm::vec3(0.0f, 2.0f, 0.0f));
	Obj tank(tank_m, glm::vec3(0.0f));
	Obj floor(floor_m, glm::vec3(0.0f, -1.55f, 0.0f));
	Cube cube1_m;
	Cube cube2_m;
	Obj cube1(cube1_m, glm::vec3(0.0f));
	Obj cube2(cube2_m, glm::vec3(0.0f, -1.55f, 0.0f));

	std::vector<IDrawable*> objs;
	objs.push_back(&sukhoi);
	objs.push_back(&tank);
	objs.push_back(&floor);

	std::vector<Light*> lights;
	lights.push_back(&dl);
	lights.push_back(&sl);
	lights.push_back(&sl2);
	lights.push_back(&pl);
	lights.push_back(&pl2);

	cube1_m.setLight(&sl);
	cube2_m.setLight(&sl2);

	Lightmanager lightmanager;
	lightmanager.addLight(&lights);
	lightmanager.addDrawable(&objs);
	std::vector<Batch*> b = lightmanager.populate(shadowshader, mainshader);
	b.push_back(new Batch(0, {&cube1, &cube2}, &cubeshader));

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
		cube1.setPosition(pos1);
		cube2.setPosition(pos2);
		cube1.rotateTowards(glm::vec3(0.0f));
		cube2.rotateTowards(glm::vec3(0.0f));
		cube1_m.setLightLocation(&pos1);
		cube2_m.setLightLocation(&pos2);

		mypath.update(0.2f, testfun);

		Info("Before render");
		for (unsigned int i = 0; i < b.size(); i++)
		{
			Info("Writing batch number " << i);
			b[i]->render();
		}

		Info("After rendering");
		window.postdraw();
	}

	glfwDestroyWindow(window.getWindow());
	glfwTerminate();
	return 0;
}
