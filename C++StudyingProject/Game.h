#pragma once
#include "DxLib.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "TutorialScene.h"  
#include "GameScene.h"
#include "LoadingScene.h"
#include "SplashScene.h"
#include "BlockAthleticsScene.h"
#include "BlockModeLoadingScene.h"  // **新規追加: ブロックモード専用ローディング**

class Game
{
public:
    Game();
    ~Game();

    // DxLibウィンドウ初期化 + Scene 初期化
    bool  Initialize();

    // メインループ（タイトル画面 or キャラクター選択 or チュートリアル or ゲーム本体 or ブロックアスレチック）
    void  Run();

    // DxLib 終了・リソース解放
    void  Finalize();

private:
    enum GameState {
        SPLASH,
        TITLE,
        CHARACTER_SELECT,
        TUTORIAL,
        BLOCK_LOADING,       // **新規追加: ブロックモード専用ローディング**
        BLOCK_MODE,
        LOADING,
        GAME_MAIN
    };

    SplashScene splashScene;
    TitleScene titleScene;
    CharacterSelectScene characterSelectScene;
    TutorialScene tutorialScene;
    LoadingScene loadingScene;
    GameScene gameScene;
    BlockAthleticsScene blockAthleticsScene;
    BlockModeLoadingScene blockModeLoadingScene;  // **新規追加**
    GameState currentState;
    int selectedCharacter;

    // ローディング関連の状態
    LoadingScene::LoadingType pendingLoadingType;
    int pendingStageIndex;
};