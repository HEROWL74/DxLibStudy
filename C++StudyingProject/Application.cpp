#include "Application.h"

Application::Application()
{

}

Application::~Application()
{
	Release();
}

bool Application::Initialize()
{
	if (DxLib_Init() == -1) return false;
	SetDrawScreen(DX_SCREEN_BACK);
	return true;
}

void Application::Update()
{
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		ClearDrawScreen();
		DrawString(100, 100, "DXlib Window", GetColor(255, 255, 255));
		ScreenFlip();
	}
}

void Application::Run()
{
	if (Initialize()) {
		Update();
	}
}

void Application::Release()
{
	DxLib_End();
}