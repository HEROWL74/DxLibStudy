#include "Application.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	std::unique_ptr<Application> app = std::make_unique<Application>();
	app->Run();
	return 0;
}