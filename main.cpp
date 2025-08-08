#include "include/Application.h"
#include "include/testlayer.h"

int main()
{
	PROFILE_BEGIN_SESSION("Application", "/tmp/application_data.json");

	Application* app = new Application();

	//app->pushLayer(new Testlayer());
	app->run();

	delete app;

	PROFILE_END_SESSION();

	return 0;
}
