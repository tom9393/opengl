#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "include/cube.h"
#include "include/Camera.h"
#include "include/obj.h"
#include "include/Window.h"
#include "include/Input.h"
#include "include/lighting.h"
#include "include/shadowmap.h"
#include "include/shadowshader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 1600;

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

	OldShader cubeshader;
	Lighting lighting;
	Shadowshader shadowshader;

	if (!lighting.Init() || !shadowshader.Init() || !cubeshader.Init())
	{
	    return 1;
	};

	PointLight pl, pl2;
	pl.setPosition(glm::vec3(0.0f, 2.5f, 0.0f));
	pl2.setPosition(glm::vec3(0.0f, 2.5f, 0.0f));

	DirLight dl;

	SpotLight sl, sl2;

	Model sukhoi_m("/home/tom/Coding/c++/opengl/assets/sukhoi.obj");
	Model tank_m("/home/tom/Coding/c++/opengl/assets/Leopard2A4.obj");
	Model floor_m("/home/tom/Coding/c++/opengl/assets/floor.obj");

	Obj sukhoi(sukhoi_m, glm::vec3(0.0f, 2.0f, 0.0f));
	Obj tank(tank_m, glm::vec3(0.0f));
	Obj floor(floor_m, glm::vec3(0.0f, -1.55f, 0.0f));

	Cube cube, cube2;
	cube.init();
	cube2.init();

	ShadowMap shadowMap, m1, m2;

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		InputHandler::process(window);
		Camera camera = InputHandler::getCamera();

		glm::vec3 lightPos = glm::vec3(5.0f * sin(glfwGetTime()), -0.25f, 5.0f * cos(glfwGetTime()));
		glm::vec3 lightPos2 = glm::vec3(0.0f, 3.0f, -1.0f * 1.0f * cos(glfwGetTime()));
		sl.setPosition(lightPos);
		sl2.setPosition(lightPos2);

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

		shadowshader.Enable();

		shadowMap.BindForWriting();
		shadowshader.setMat4("lightSpaceMatrix", dl.getMatrix());
		sukhoi.draw(shadowshader);
		tank.draw(shadowshader);
		floor.draw(shadowshader);

		m1.BindForWriting();
		shadowshader.setMat4("lightSpaceMatrix", sl.getMatrix());
		sukhoi.draw(shadowshader);
		tank.draw(shadowshader);
		floor.draw(shadowshader);

		m2.BindForWriting();
		shadowshader.setMat4("lightSpaceMatrix", sl2.getMatrix());
		sukhoi.draw(shadowshader);
		tank.draw(shadowshader);
		floor.draw(shadowshader);

		// Reset
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubeshader.Enable();

		glm::vec3 asdf = glm::vec3(0.0f, 5.0f, 0.0f);

		pl.setPosition(asdf);
		pl2.setPosition(asdf);

		cube.setLocation(lightPos);
		cube.setRotationTarget(glm::vec3(0.0f));
		cube.draw(cubeshader);

		cube2.setLocation(lightPos2);
		cube2.setRotationTarget(glm::vec3(0.0f));
		cube2.draw(cubeshader);

		sl.setPosition(lightPos);
		sl.setDirection(sl.getDirectionToTarget(sl.getMatrix()));

		sl2.setPosition(lightPos2);
		sl2.setDirection(sl2.getDirectionToTarget(sl2.getMatrix()));

		lighting.Enable();
		lighting.setPointlight(0, pl);
		// lighting.setPointlight(1, pl2);
		lighting.setDirlight(0, dl);
		lighting.setSpotlight(0, sl);
		lighting.setSpotlight(1, sl2);

		lighting.SetCameraLocalPos(camera.Position);
		shadowMap.BindForReading(GL_TEXTURE30);
		lighting.setInt("shadowMap", 30);
		lighting.setMat4("lightSpaceMatrix", dl.getMatrix());
		m1.BindForReading(GL_TEXTURE28);
		lighting.setInt("shadowMapSpot[0]", 28);
		m2.BindForReading(GL_TEXTURE29);
		lighting.setInt("shadowMapSpot[1]", 29);
		lighting.setMat4("spotLightSpaceMatrices[0]", sl.getMatrix());
		lighting.setMat4("spotLightSpaceMatrices[1]", sl2.getMatrix());

		lighting.SetView(view);
		lighting.SetProjection(projection);
	
		tank.draw(lighting);
		sukhoi.draw(lighting);
		floor.draw(lighting);

		glfwSwapBuffers(window);
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
