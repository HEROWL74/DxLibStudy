#include "Game.h"

Game::Game() : currentState(TITLE), selectedCharacter(-1) {}
Game::~Game() {}

bool Game::Initialize()
{
    // ウィンドウ設定：1920×1080
    ChangeWindowMode(TRUE);
    SetGraphMode(1920, 1080, 32);
    if (DxLib_Init() == -1) return false;
    SetDrawScreen(DX_SCREEN_BACK);

    // シーン初期化
    titleScene.Initialize();
    characterSelectScene.Initialize();
    // gameSceneは選択されたキャラクターで初期化するため、ここでは初期化しない
    return true;
}

void Game::Run()
{
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        ClearDrawScreen();

        switch (currentState) {
        case TITLE:
            titleScene.Update();
            titleScene.Draw();

            // 画面遷移完了チェック
            if (titleScene.IsTransitionComplete()) {
                currentState = CHARACTER_SELECT;
                titleScene.ResetTransition();
            }
            else if (titleScene.IsExitRequested()) {
                return; // ゲーム終了
            }
            break;

        case CHARACTER_SELECT:
            characterSelectScene.Update();
            characterSelectScene.Draw();

            if (characterSelectScene.IsCharacterSelected()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();
                currentState = GAME_MAIN;
                // ゲームシーンを選択されたキャラクターで初期化
                gameScene.Initialize(selectedCharacter);
                // キャラクター選択画面の状態をリセット
                characterSelectScene.ResetState();
            }
            else if (characterSelectScene.IsBackRequested()) {
                currentState = TITLE;
                // キャラクター選択画面の状態をリセット
                characterSelectScene.ResetState();
            }
            break;

        case GAME_MAIN:
            gameScene.Update();
            gameScene.Draw();

            if (gameScene.IsExitRequested()) {
                currentState = TITLE;
            }
            break;
        }

        ScreenFlip();
    }
}

void Game::Finalize()
{
    DxLib_End();
}