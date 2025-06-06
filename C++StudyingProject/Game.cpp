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
    // �E�B���h�E�ݒ�F1920�~1080
    ChangeWindowMode(TRUE);
    SetGraphMode(1920, 1080, 32);
    if (DxLib_Init() == -1) return false;
    SetDrawScreen(DX_SCREEN_BACK);

    // �V�[��������
    titleScene.Initialize();
    characterSelectScene.Initialize();
    loadingScene.Initialize();  // �V�K�ǉ�
    // gameScene�͑I�����ꂽ�L�����N�^�[�ŏ��������邽�߁A�����ł͏��������Ȃ�

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

            // ��ʑJ�ڊ����`�F�b�N
            if (titleScene.IsTransitionComplete()) {
                currentState = CHARACTER_SELECT;
                titleScene.ResetTransition();

                OutputDebugStringA("Game: Switched to CHARACTER_SELECT - BGM stopped\n");
            }
            else if (titleScene.IsExitRequested()) {
                // **�Q�[���I���F�S������~**
                SoundManager::GetInstance().StopBGM();
                SoundManager::GetInstance().StopAllSE();
                return; // �Q�[���I��
            }
            break;

        case CHARACTER_SELECT:
            characterSelectScene.Update();
            characterSelectScene.Draw();

            if (characterSelectScene.IsCharacterSelected()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();

                // **���[�f�B���O��ʂ��J�n**
                currentState = LOADING;
                pendingLoadingType = LoadingScene::LOADING_GAME_START;
                loadingScene.StartLoading(pendingLoadingType, selectedCharacter, 0);

                SoundManager::GetInstance().StopBGM();

                // **�L�����N�^�[�I����ʂ̏�Ԃ����Z�b�g**
                characterSelectScene.ResetState();

                OutputDebugStringA("Game: Started loading for GAME_MAIN\n");
            }
            else if (characterSelectScene.IsBackRequested()) {
                currentState = TITLE;

                // **�^�C�g����ʂɖ߂�F�^�C�g��BGM���ĊJ**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

                // **�L�����N�^�[�I����ʂ̏�Ԃ����Z�b�g**
                characterSelectScene.ResetState();

                OutputDebugStringA("Game: Returned to TITLE - Title BGM restarted\n");
            }
            break;

        case LOADING:  // **�V�K�ǉ��F���[�f�B���O���**
            loadingScene.Update();
            loadingScene.Draw();

            if (loadingScene.IsLoadingComplete()) {
                // ���[�f�B���O������̑J��
                if (pendingLoadingType == LoadingScene::LOADING_GAME_START) {
                    currentState = GAME_MAIN;

                    // **�Q�[���V�[����I�����ꂽ�L�����N�^�[�ŏ�����**
                    gameScene.Initialize(selectedCharacter);

                    OutputDebugStringA("Game: Loading complete, switched to GAME_MAIN\n");
                }
                else if (pendingLoadingType == LoadingScene::LOADING_STAGE_CHANGE) {
                    // �X�e�[�W�ύX�̏ꍇ��GAME_MAIN�ɖ߂�
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

                // **�Q�[������^�C�g���ɖ߂�F�^�C�g��BGM���ĊJ**
                SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

                OutputDebugStringA("Game: Returned to TITLE from GAME - Title BGM restarted\n");
            }
            // **�V�@�\�F�X�e�[�W�ύX���̃��[�f�B���O�i��FTAB�L�[�ł̃X�e�[�W�؂�ւ��j**
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