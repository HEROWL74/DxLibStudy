#pragma once
#include "DxLib.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"

class Game
{
public:
    Game();
    ~Game();

    // DxLib・ウィンドウ初期化 + Scene 初期化
    bool  Initialize();

    // メインループ（タイトル画面 or キャラクター選択 or ゲーム本体）
    void  Run();

    // DxLib 終了・リソース開放
    void  Finalize();

private:
    enum GameState { TITLE, CHARACTER_SELECT, GAME_MAIN };

    TitleScene titleScene;
    CharacterSelectScene characterSelectScene;
    GameScene gameScene;
    GameState currentState;
    int selectedCharacter;
};