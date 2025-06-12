#pragma once
#include "DxLib.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "TutorialScene.h"  // **新追加**
#include "GameScene.h"
#include "LoadingScene.h"
#include "SplashScene.h"

class Game
{
public:
    Game();
    ~Game();

    // DxLibウィンドウ初期化 + Scene 初期化
    bool  Initialize();

    // メインループ（タイトル画面 or キャラクター選択 or チュートリアル or ゲーム本体）
    void  Run();

    // DxLib 終了・リソース開放
    void  Finalize();

private:
    enum GameState {
        SPLASH,
        TITLE,
        CHARACTER_SELECT,
        TUTORIAL,        // **新追加**
        LOADING,
        GAME_MAIN
    };

    SplashScene splashScene;
    TitleScene titleScene;
    CharacterSelectScene characterSelectScene;
    TutorialScene tutorialScene;  // **新追加**
    LoadingScene loadingScene;
    GameScene gameScene;
    GameState currentState;
    int selectedCharacter;

    // ローディング関連の状態
    LoadingScene::LoadingType pendingLoadingType;
    int pendingStageIndex;
};