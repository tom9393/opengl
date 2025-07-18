#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/cubeshader.h"
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

	Transform* dlt = new Transform(glm::vec3(2.0f, 10.0f, -1.0f));
	Transform* plt = new Transform(glm::vec3(50.0f));
	Transform* plt2 = new Transform(glm::vec3(50.0f));
	Transform* slt = new Transform(glm::vec3(1.0f));
	Transform* slt2 = new Transform(glm::vec3(5.0f));

	DirectionalLight dl(*dlt);
	PointLight pl(*plt);
	PointLight pl2(*plt2);
	SpotLight sl(*slt);
	SpotLight sl2(*slt2);

	Transform* trans1 = new Transform(glm::vec3(0.0f, 2.0f, 0.0f));
	Transform* trans2 = new Transform(glm::vec3(0.0f));
	Transform* trans3 = new Transform(glm::vec3(0.0f, -1.55f, 0.0f));

	Transform* transcube1 = new Transform(glm::vec3(0.0f, -1.55f, 0.0f));
	Transform* transcube2 = new Transform(glm::vec3(0.0f, -1.55f, 0.0f));

	Model sukhoi("/home/tom/Coding/c++/opengl/assets/sukhoi.obj", *trans1);
	Model tank("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj", *trans2);
	Model floor("/home/tom/Coding/c++/opengl/assets/floor.obj", *trans3);
	Cube cube1(*transcube1);
	Cube cube2(*transcube2);

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

	std::vector<IDrawable*> cubes;
	cubes.push_back(&cube1);
	cubes.push_back(&cube2);

	Lightmanager lightmanager;
	lightmanager.addLight(&lights);
	lightmanager.addDrawable(&objs);
	std::vector<Batch*> b = lightmanager.populate(shadowshader, mainshader);
	b.push_back(new Batch(0, cubes, &cubeshader));

	std::vector<glm::vec3> thingeys;

	for (unsigned int i = 0; i < 20; i++) thingeys.push_back(glm::vec3(sin(i) * i, 2.0f, cos(i) * i));

	Path mypath(&sukhoi);
	mypath.setPath(thingeys);

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		window.predraw();
		float x = sin(glfwGetTime()) * 5.0f;
		float y = cos(glfwGetTime()) * 5.0f;
		glm::vec3 pos1 = glm::vec3(x * -1.0f, -0.25f, y);
		glm::vec3 pos2 = glm::vec3(x, 3.0f, y);
		cube1.rotateTowards(glm::vec3(0.0f));
		cube2.rotateTowards(glm::vec3(0.0f));
		cube1.setPosition(pos1);
		cube2.setPosition(pos2);
		sl.setPosition(pos1);
		sl2.setPosition(pos2);
		sl.rotateTowards(glm::vec3(0.0f));
		sl2.rotateTowards(glm::vec3(0.0f));

		mypath.update(0.2f, testfun);
		tank.rotateY(0.01f);

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
