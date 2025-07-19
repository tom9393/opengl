#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/cubeshader.h"
#include "include/instance.h"
#include "include/lightmanager.h"
#include "include/model.h"
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
	std::cout << "Loop n. " << n << std::endl;
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

	DirectionalLight dl(glm::vec3(2.0f, 10.0f, -1.0f));
	PointLight pl(glm::vec3(50.0f));
	PointLight pl2(glm::vec3(50.0f));
	SpotLight sl(glm::vec3(1.0f));
	SpotLight sl2(glm::vec3(5.0f));

	Model sukhoi("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor("/home/tom/Coding/c++/opengl/assets/floor.obj");
	Cube cube1(glm::vec3(0.0f, -1.55f, 0.0f));
	Cube cube2(glm::vec3(0.0f, -1.55f, 0.0f));

	Instance isukhoi(sukhoi, glm::vec3(0.0f, 2.0f, 0.0f));
	Instance isukhoi2(sukhoi, glm::vec3(0.0f, 5.0f, 0.0f));
	Instance itank(tank, glm::vec3(0.0f));
	Instance ifloor(floor, glm::vec3(0.0f, -1.55f, 0.0f));

	std::vector<IDrawable*> objs = { &isukhoi, &isukhoi2, &itank, &ifloor };
	std::vector<Light*> lights = { &dl, &sl, &sl2, &pl, &pl2 };
	std::vector<IDrawable*> cubes = { &cube1, &cube2 };

	Lightmanager lightmanager;
	lightmanager.addLight(&lights);
	lightmanager.addDrawable(&objs);
	std::vector<Batch*> b = lightmanager.populate(shadowshader, mainshader);
	b.push_back(new Batch(0, cubes, &cubeshader));

	std::vector<glm::vec3> pathing;

	for (unsigned int i = 0; i < 20; i++) pathing.push_back(glm::vec3(sin(i) * i, 2.0f, cos(i) * i));

	Path mypath(&isukhoi);
	mypath.setPath(pathing);

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		window.predraw();
		float x = sin(glfwGetTime()) * 5.0f;
		float y = cos(glfwGetTime()) * 5.0f;
		glm::vec3 pos1 = glm::vec3(x * -1.0f, -0.25f, y);
		glm::vec3 pos2 = glm::vec3(x, 2.0f, y);
		Transform::rotateTowards(cube1, glm::vec3(0.0f));
		Transform::rotateTowards(cube2, glm::vec3(0.0f));
		Transform::rotateTowards(sl, glm::vec3(0.0f));
		Transform::rotateTowards(sl2, glm::vec3(0.0f));
		Transform::setPosition(cube1, pos1);
		Transform::setPosition(cube2, pos2);
		Transform::setPosition(sl, pos1);
		Transform::setPosition(sl2, pos2);

		mypath.update(0.2f, testfun);
		Transform::rotateY(itank, 0.01f);

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
