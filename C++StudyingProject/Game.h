#pragma once
#include "DxLib.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"
#include "LoadingScene.h"
#include "SplashScene.h"

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
    enum GameState { SPLASH, TITLE, CHARACTER_SELECT, LOADING, GAME_MAIN };

    SplashScene splashScene;
    TitleScene titleScene;
    CharacterSelectScene characterSelectScene;
    LoadingScene loadingScene;
    GameScene gameScene;
    GameState currentState;
    int selectedCharacter;

    // ���[�f�B���O�֘A�̏��
    LoadingScene::LoadingType pendingLoadingType;
    int pendingStageIndex;

};