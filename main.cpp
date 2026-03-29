#include "src/Application.h"

const bool editor = true;

int main()
{
	PROFILE_BEGIN_SESSION("Application", "/tmp/application_data.json");

	Engine::Application* app;

	if (editor)
	{
		app = new Engine::Editor();
	} else
	{
		app = new Engine::Runtime();
	}

	app->Run();
	
	delete app;

	PROFILE_END_SESSION();

	return 0;
}
