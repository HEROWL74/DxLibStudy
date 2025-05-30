#include "Game.h"

Game::Game() : currentState(TITLE), selectedCharacter(-1) {}
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
    // gameScene�͑I�����ꂽ�L�����N�^�[�ŏ��������邽�߁A�����ł͏��������Ȃ�
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
            }
            else if (titleScene.IsExitRequested()) {
                return; // �Q�[���I��
            }
            break;

        case CHARACTER_SELECT:
            characterSelectScene.Update();
            characterSelectScene.Draw();

            if (characterSelectScene.IsCharacterSelected()) {
                selectedCharacter = characterSelectScene.GetSelectedCharacter();
                currentState = GAME_MAIN;
                // �Q�[���V�[����I�����ꂽ�L�����N�^�[�ŏ�����
                gameScene.Initialize(selectedCharacter);
                // �L�����N�^�[�I����ʂ̏�Ԃ����Z�b�g
                characterSelectScene.ResetState();
            }
            else if (characterSelectScene.IsBackRequested()) {
                currentState = TITLE;
                // �L�����N�^�[�I����ʂ̏�Ԃ����Z�b�g
                characterSelectScene.ResetState();
            }
            break;

        case GAME_MAIN:
            gameScene.Update();
            gameScene.Draw();

            if (gameScene.IsExitRequested()) {
                currentState = TITLE;
            }
            break;
        }

        ScreenFlip();
    }
}

void Game::Finalize()
{
    DxLib_End();
}