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

    // DxLib�E�E�B���h�E������ + Scene ������
    bool  Initialize();

    // ���C�����[�v�i�^�C�g����� or �L�����N�^�[�I�� or �Q�[���{�́j
    void  Run();

    // DxLib �I���E���\�[�X�J��
    void  Finalize();

private:
    enum GameState { TITLE, CHARACTER_SELECT, GAME_MAIN };

    TitleScene titleScene;
    CharacterSelectScene characterSelectScene;
    GameScene gameScene;
    GameState currentState;
    int selectedCharacter;
};