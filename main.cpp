#include "include/Application.h"
#include "include/testlayer.h"

int main()
{
	Application app;
	app.pushLayer(new Testlayer());
	app.run();

	return 0;
}
