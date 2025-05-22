#include "Application.h"
#include <memory>


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
    std::unique_ptr<Application> app = std::make_unique<Application>();
    app->Run();
    return 0;
}
