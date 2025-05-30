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
    // �Q�[�����
    GameState currentState;

    // �e�}�l�[�W���[
    EffectManager* effectManager;
    UIManager* uiManager;
    SoundManager* soundManager;

    // �w�i�摜�n���h��
    int backgroundHandle;

    // �^�C�}�[�n
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