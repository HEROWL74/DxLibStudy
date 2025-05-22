#include "Application.h"
#include "DxLib.h"
#include "TitleScene.h"
#include "GameScene.h"

Application::Application() {}

Application::~Application() {
    Release();
}

bool Application::Initialize() {
    if (DxLib_Init() == -1) {
        return false;
    }
    SetDrawScreen(DX_SCREEN_BACK);

    currentScene = SceneType::Title;
    title = std::make_unique<TitleScene>();
    return true;
}

void Application::Update() {
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
        ClearDrawScreen();

        switch (currentScene) {
        case SceneType::Title:
            title->Update();
            title->Draw();
            if (title->IsStartRequested()) {
                game = std::make_unique<GameScene>();
                currentScene = SceneType::Game;
                title.reset();
            }
            break;
        case SceneType::Game:
            game->Update();
            game->Draw();
            break;
        }

        ScreenFlip();
    }
}

void Application::Release() {
    title.reset();
    game.reset();
    DxLib_End();
}

void Application::Run() {
    if (Initialize()) {
        Update();
    }
}
