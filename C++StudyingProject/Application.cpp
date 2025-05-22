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

	m_game = std::make_unique<Game>(); //Game初期化

	return true;
}

void Application::Update()
{
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		ClearDrawScreen();
		DrawString(300, 100, "DXlib Window", GetColor(255, 255, 255));
		if (m_game) {
			m_game->Update();
			m_game->Draw();
		}

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
	m_game.reset(); // 明示的に開放する（optional）
	DxLib_End();
}