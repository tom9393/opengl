#include "include/cube.h"
#include "include/cubeshader.h"
#include "include/instance.h"
#include "include/lightmanager.h"
#include "include/pathfollower.h"
#include "include/rng.h"
#include "include/model.h"
#include "include/Window.h"
#include "include/mainshader.h"
#include "include/path.h"
#include "include/shadowshader.h"
#include "include/preprocessor.h"

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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	Mainshader mainshader;
	Shadowshader shadowshader;
	Cubeshader cubeshader;

	mainshader.setView(window.getCamera());
	shadowshader.setView(window.getCamera());
	cubeshader.setView(window.getCamera());

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
	Instance itank(tank, glm::vec3(0.0f));
	Instance ifloor(floor, glm::vec3(0.0f, -1.55f, 0.0f));

	std::vector<IDrawable*> objs = { &isukhoi, &itank, &ifloor };
	std::vector<Light*> lights = { &dl, &sl, &sl2, &pl, &pl2 };
	std::vector<IDrawable*> cubes = { &cube1, &cube2 };

	RandomGenerator* rng = &RandomGenerator::getInstance();

	std::vector<PathFollower*> followers;

	for (int i = 0; i < 3; ++i) {
		IDrawable* inst = new Instance(sukhoi, glm::vec3(1.0f));
		objs.push_back(inst);

		std::vector<glm::vec3> points;
		for (int i = 0; i < 10; ++i) {
			float x = RandomGenerator::getInstance().getFloat(-10.0f, 10.0f);
			float y = RandomGenerator::getInstance().getFloat(0.5f, 10.0f);
			float z = RandomGenerator::getInstance().getFloat(-10.0f, 10.0f);
			points.push_back(glm::vec3(x, y, z));
		}

		Path* path = new Path(points); // looping
		Transformable* tf = dynamic_cast<Transformable*>(inst);
		PathFollower* follower = new PathFollower(tf, path, 5.0f, glm::radians(360.0f));
		followers.push_back(follower);
	}

	Lightmanager lightmanager;
	lightmanager.addLight(&lights);
	lightmanager.addDrawable(&objs);
	std::vector<Batch*> b = lightmanager.populate(shadowshader, mainshader);
	b.push_back(new Batch(0, cubes, &cubeshader));

	float deltaTime = 0.016f;

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		float x = sin(glfwGetTime()) * 5.0f;
		float y = cos(glfwGetTime()) * 5.0f;
		glm::vec3 pos1 = glm::vec3(x * -1.0f, -0.25f, y);
		glm::vec3 pos2 = glm::vec3(x, 2.0f, y);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (window.imgui())
		{
			ImGui::ShowDemoWindow(); // Show demo window! :)
			ImGui::Begin("Create a new instance");
			ImGui::SliderFloat("float", &deltaTime, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			if (ImGui::Button("Spawn another"))
			{
				std::cout << "Pressed the button" << std::endl;
				Cube cubet(glm::vec3(x, -1.55f, y));
				std::vector<IDrawable*> gaa;
				gaa.push_back(&cubet);
				b.push_back(new Batch(0, gaa, &cubeshader));

			}
			ImGui::End();
		}
		window.predraw();
		Transform::rotateTowards(cube1, itank);
		Transform::rotateTowards(cube2, itank);
		Transform::rotateTowards(sl, itank);
		Transform::rotateTowards(sl2, itank);
		Transform::setPosition(cube1, pos1);
		Transform::setPosition(cube2, pos2);
		Transform::setPosition(sl, pos1);
		Transform::setPosition(sl2, pos2);

		Transform::rotateY(itank, deltaTime);

		for (auto& f : followers)
		    f->update(deltaTime);

		Info("Before render");
		for (unsigned int i = 0; i < b.size(); i++)
		{
			Info("Writing batch number " << i);
			b[i]->render();
		}
		Info("After rendering");
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		window.postdraw();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window.getWindow());
	glfwTerminate();
	return 0;
}
