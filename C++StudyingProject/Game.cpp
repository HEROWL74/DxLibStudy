#include "Game.h"

Game::Game()
    : currentState(TITLE)
    , selectedCharacter(-1)
    , pendingLoadingType(LoadingScene::LOADING_GAME_START)
    , pendingStageIndex(-1)
{}

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
    loadingScene.Initialize();  // 新規追加
    // gameSceneは選択されたキャラクターで初期化するため、ここでは初期化しない

    SoundManager::GetInstance().Initialize();

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

                OutputDebugStringA("Game: Switched to CHARACTER_SELECT - BGM stopped\n");
            }
            else if (titleScene.IsExitRequested()) {
                // **ゲーム終了：全音声停止**
                SoundManager::GetInstance().StopBGM();
                SoundManager::GetInstance().StopAllSE();
                return; // ゲーム終了
            }
            break;

        case CHARACTER_SELECT:
            characterSelectScene.Update();
            characterSelectScene.Draw();

            if (characterSelectScene.IsCharacterSelected()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();

                // **ローディング画面を開始**
                currentState = LOADING;
                pendingLoadingType = LoadingScene::LOADING_GAME_START;
                loadingScene.StartLoading(pendingLoadingType, selectedCharacter, 0);

                SoundManager::GetInstance().StopBGM();

                // **キャラクター選択画面の状態をリセット**
                characterSelectScene.ResetState();

                OutputDebugStringA("Game: Started loading for GAME_MAIN\n");
            }
            else if (characterSelectScene.IsBackRequested()) {
                currentState = TITLE;

                // **タイトル画面に戻る：タイトルBGMを再開**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

                // **キャラクター選択画面の状態をリセット**
                characterSelectScene.ResetState();

                OutputDebugStringA("Game: Returned to TITLE - Title BGM restarted\n");
            }
            break;

        case LOADING:  // **新規追加：ローディング状態**
            loadingScene.Update();
            loadingScene.Draw();

            if (loadingScene.IsLoadingComplete()) {
                // ローディング完了後の遷移
                if (pendingLoadingType == LoadingScene::LOADING_GAME_START) {
                    currentState = GAME_MAIN;

                    // **ゲームシーンを選択されたキャラクターで初期化**
                    gameScene.Initialize(selectedCharacter);

                    OutputDebugStringA("Game: Loading complete, switched to GAME_MAIN\n");
                }
                else if (pendingLoadingType == LoadingScene::LOADING_STAGE_CHANGE) {
                    // ステージ変更の場合はGAME_MAINに戻る
                    currentState = GAME_MAIN;

                    OutputDebugStringA("Game: Stage loading complete\n");
                }
            }
            break;

        case GAME_MAIN:
            gameScene.Update();
            gameScene.Draw();

            if (gameScene.IsExitRequested()) {
                currentState = TITLE;

                // **ゲームからタイトルに戻る：タイトルBGMを再開**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

                OutputDebugStringA("Game: Returned to TITLE from GAME - Title BGM restarted\n");
            }
            // **新機能：ステージ変更時のローディング（例：TABキーでのステージ切り替え）**
            else if (gameScene.IsStageChangeRequested()) {
                currentState = LOADING;
                pendingLoadingType = LoadingScene::LOADING_STAGE_CHANGE;
                pendingStageIndex = gameScene.GetRequestedStageIndex();

                loadingScene.StartLoading(pendingLoadingType, selectedCharacter, pendingStageIndex);

                OutputDebugStringA("Game: Started stage change loading\n");
            }
            break;
        }

        ScreenFlip();
    }
}

void Game::Finalize()
{
    SoundManager::GetInstance().Finalize();

    DxLib_End();
}