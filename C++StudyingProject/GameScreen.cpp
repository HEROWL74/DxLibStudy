#include "GameScreen.h"

GameScreen::GameScreen()
{
    currentState = GAME_STATE_START;
    effectManager = nullptr;
    uiManager = nullptr;
    soundManager = nullptr;
    backgroundHandle = -1;
    frameCounter = 0;
}

GameScreen::~GameScreen()
{
    Finalize();
}

void GameScreen::Initialize()
{
    // �e�}�l�[�W���[�̏�����
    effectManager = new EffectManager();
    uiManager = new UIManager();
    soundManager = new SoundManager();

    effectManager->Initialize();
    uiManager->Initialize();
    soundManager->Initialize();

    // �w�i�摜�̓ǂݍ���
    backgroundHandle = LoadGraph("Sprites/Backgrounds/mountain_background.png");

    // ������Ԃ��X�^�[�g��ʂɐݒ�
    ChangeState(GAME_STATE_START);
}

void GameScreen::Update()
{
    frameCounter++;

    switch (currentState)
    {
    case GAME_STATE_START:
        // �X�^�[�g��ʂ̍X�V
        uiManager->UpdateStartScreen();
        effectManager->Update();

        // �X�^�[�g�{�^���������ꂽ��Q�[���J�n
        if (uiManager->IsStartButtonPressed())
        {
            ChangeState(GAME_STATE_PLAYING);
        }
        break;

    case GAME_STATE_PLAYING:
        // �Q�[�����̏���
        // ���̓X�^�[�g��ʂ̃f���Ȃ̂ŊȈՎ���
        break;

    case GAME_STATE_PAUSE:
        // �|�[�Y��ʂ̏���
        break;

    case GAME_STATE_GAMEOVER:
        // �Q�[���I�[�o�[��ʂ̏���
        break;
    }
}

void GameScreen::Draw()
{
    switch (currentState)
    {
    case GAME_STATE_START:
        // �w�i�`��
        if (backgroundHandle != -1)
        {
            DrawExtendGraph(0, 0, 1280, 720, backgroundHandle, TRUE);
        }

        // �G�t�F�N�g�`��
        effectManager->Draw();

        // UI�`��
        uiManager->DrawStartScreen();
        break;

    case GAME_STATE_PLAYING:
        // �Q�[�����̕`��
        break;

    case GAME_STATE_PAUSE:
        // �|�[�Y��ʂ̕`��
        break;

    case GAME_STATE_GAMEOVER:
        // �Q�[���I�[�o�[��ʂ̕`��
        break;
    }
}

void GameScreen::ChangeState(GameState newState)
{
    currentState = newState;

    // ��ԕύX���̏���
    switch (newState)
    {
    case GAME_STATE_START:
        effectManager->StartTitleEffects();
        soundManager->PlayBGM("title");
        break;

    case GAME_STATE_PLAYING:
        soundManager->PlayBGM("game");
        break;
    }
}

void GameScreen::Finalize()
{
    // ���\�[�X�̉��
    if (backgroundHandle != -1)
    {
        DeleteGraph(backgroundHandle);
        backgroundHandle = -1;
    }

    // �}�l�[�W���[�̏I������
    if (effectManager)
    {
        effectManager->Finalize();
        delete effectManager;
        effectManager = nullptr;
    }

    if (uiManager)
    {
        uiManager->Finalize();
        delete uiManager;
        uiManager = nullptr;
    }

    if (soundManager)
    {
        soundManager->Finalize();
        delete soundManager;
        soundManager = nullptr;
    }
}