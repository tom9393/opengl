#include "include/Application.h"
#include "include/ImGuiLayer.h"
#include "include/testlayer.h"

int main()
{
	Application app;
	app.pushLayer(new Testlayer());
	app.pushOverlayer(new ImGuiLayer());
	app.run();

	return 0;
}
