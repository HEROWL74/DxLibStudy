#pragma once
#include "DxLib.h"
#include "EffectManager.h"
#include "UIManager.h"
#include "SoundManager.h"

enum GameState
{
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSE,
    GAME_STATE_GAMEOVER
};

class GameScreen
{
private:
    // ゲーム状態
    GameState currentState;

    // 各マネージャー
    EffectManager* effectManager;
    UIManager* uiManager;
    SoundManager* soundManager;

    // 背景画像ハンドル
    int backgroundHandle;

    // タイマー系
    int frameCounter;

public:
    GameScreen();
    ~GameScreen();

    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    void ChangeState(GameState newState);
    GameState GetCurrentState() const { return currentState; }
};