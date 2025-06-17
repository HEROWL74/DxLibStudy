#pragma once
#include "DxLib.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "TutorialScene.h"  
#include "GameScene.h"
#include "LoadingScene.h"
#include "SplashScene.h"
#include "BlockAthleticsScene.h"
#include "BlockModeLoadingScene.h"  // **�V�K�ǉ�: �u���b�N���[�h��p���[�f�B���O**

class Game
{
public:
    Game();
    ~Game();

    // DxLib�E�B���h�E������ + Scene ������
    bool  Initialize();

    // ���C�����[�v�i�^�C�g����� or �L�����N�^�[�I�� or �`���[�g���A�� or �Q�[���{�� or �u���b�N�A�X���`�b�N�j
    void  Run();

    // DxLib �I���E���\�[�X���
    void  Finalize();

private:
    enum GameState {
        SPLASH,
        TITLE,
        CHARACTER_SELECT,
        TUTORIAL,
        BLOCK_LOADING,       // **�V�K�ǉ�: �u���b�N���[�h��p���[�f�B���O**
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
    BlockModeLoadingScene blockModeLoadingScene;  // **�V�K�ǉ�**
    GameState currentState;
    int selectedCharacter;

    // ���[�f�B���O�֘A�̏��
    LoadingScene::LoadingType pendingLoadingType;
    int pendingStageIndex;
};