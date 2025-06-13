#include "Game.h"

Game::Game()
    : currentState(SPLASH)
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
    splashScene.Initialize();
    titleScene.Initialize();
    characterSelectScene.Initialize();
    tutorialScene.Initialize(0);  // **新追加：仮の初期化**
    loadingScene.Initialize();

    SoundManager::GetInstance().Initialize();

    return true;
}

void Game::Run()
{
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        ClearDrawScreen();

        switch (currentState) {
        case SPLASH: {
            splashScene.Update();
            splashScene.Draw();

            if (splashScene.IsTransitionComplete()) {
                currentState = TITLE;
                splashScene.ResetTransition();
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);
                OutputDebugStringA("Game: Switched from SPLASH to TITLE\n");
            }
            break;
        }
        case TITLE: {
            titleScene.Update();
            titleScene.Draw();

            if (titleScene.IsTransitionComplete()) {
                currentState = CHARACTER_SELECT;
                titleScene.ResetTransition();
                OutputDebugStringA("Game: Switched to CHARACTER_SELECT - BGM stopped\n");
            }
            else if (titleScene.IsExitRequested()) {
                SoundManager::GetInstance().StopBGM();
                SoundManager::GetInstance().StopAllSE();
                return;
            }
            break;
        }

        case CHARACTER_SELECT: {
            // **タイトルからの設定をキャラクター選択に反映**
            static bool tutorialSettingApplied = false;
            if (!tutorialSettingApplied) {
                characterSelectScene.SetTutorialEnabled(titleScene.IsTutorialEnabled());
                tutorialSettingApplied = true;
            }

            characterSelectScene.Update();
            characterSelectScene.Draw();

            if (characterSelectScene.IsCharacterSelected()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();
                currentState = LOADING;
                pendingLoadingType = LoadingScene::LOADING_GAME_START;
                loadingScene.StartLoading(pendingLoadingType, selectedCharacter, 0);
                SoundManager::GetInstance().StopBGM();
                characterSelectScene.ResetState();
                tutorialSettingApplied = false; // **リセット時にフラグもクリア**
                OutputDebugStringA("Game: Started loading for GAME_MAIN\n");
            }
            else if (characterSelectScene.IsTutorialRequested()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();
                currentState = TUTORIAL;
                tutorialScene.Initialize(selectedCharacter);
                SoundManager::GetInstance().StopBGM();
                characterSelectScene.ResetState();
                tutorialSettingApplied = false;
                OutputDebugStringA("Game: Started TUTORIAL\n");
            }
            else if (characterSelectScene.IsBackRequested()) {
                currentState = TITLE;
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);
                characterSelectScene.ResetState();
                tutorialSettingApplied = false; // **リセット時にフラグもクリア**
                OutputDebugStringA("Game: Returned to TITLE\n");
            }
            break;
        }

        case TUTORIAL: {
            tutorialScene.Update();
            tutorialScene.Draw();

            if (tutorialScene.IsCompleted()) {
                // **チュートリアル完了後は必ずキャラクター選択に戻る**
                currentState = CHARACTER_SELECT;
                characterSelectScene.SetTutorialEnabled(titleScene.IsTutorialEnabled()); // **設定を再適用**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);
                OutputDebugStringA("Game: Tutorial completed, returned to CHARACTER_SELECT\n");
            }
            else if (tutorialScene.IsExitRequested()) {
                // **チュートリアル中断時もキャラクター選択に戻る**
                currentState = CHARACTER_SELECT;
                characterSelectScene.SetTutorialEnabled(titleScene.IsTutorialEnabled()); // **設定を再適用**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);
                OutputDebugStringA("Game: Returned to CHARACTER_SELECT from TUTORIAL\n");
            }
            break;
        }

        case LOADING: {
            loadingScene.Update();
            loadingScene.Draw();

            if (loadingScene.IsLoadingComplete()) {
                if (pendingLoadingType == LoadingScene::LOADING_GAME_START) {
                    currentState = GAME_MAIN;
                    gameScene.Initialize(selectedCharacter);

                    OutputDebugStringA("Game: Loading complete, switched to GAME_MAIN\n");
                }
                else if (pendingLoadingType == LoadingScene::LOADING_STAGE_CHANGE) {
                    currentState = GAME_MAIN;
                    OutputDebugStringA("Game: Stage loading complete\n");
                }
            }
            break;
        }

        case GAME_MAIN: {
            gameScene.Update();

            // **描画は常に実行（ポーズUI表示のため）**
            gameScene.Draw();

            // **ゲーム終了リクエスト処理**
            if (gameScene.IsExitRequested()) {
                currentState = TITLE;

                // **音声管理**
                SoundManager::GetInstance().StopBGM();
                SoundManager::GetInstance().StopAllSE();
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

                OutputDebugStringA("Game: Returned to TITLE from GAME - Title BGM restarted\n");
            }
            // **ステージ変更リクエスト処理**
            else if (gameScene.IsStageChangeRequested()) {
                currentState = LOADING;
                pendingLoadingType = LoadingScene::LOADING_STAGE_CHANGE;
                pendingStageIndex = gameScene.GetRequestedStageIndex();

                loadingScene.StartLoading(pendingLoadingType, selectedCharacter, pendingStageIndex);

                OutputDebugStringA("Game: Started stage change loading\n");
            }
            break;
        }
        }

        ScreenFlip();
    }
}

void Game::Finalize()
{
    SoundManager::GetInstance().Finalize();
    DxLib_End();
}